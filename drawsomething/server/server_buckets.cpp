#include "buckets.h"

#include "stb_divide.h"
#include "tinker_platform.h"

#include "ds_server.h"
#include "game/s_artist.h"

void ServerBuckets::AddNewStroke(net_peer_t from_peer)
{
	ServerArtist* server_artist = &g_server_data->m_server_artists[from_peer];
	server_artist->m_current_stroke.Invalidate();
}

void ServerBuckets::AddPointToStroke(net_peer_t from_peer, vec3* point)
{
	BucketCoordinate bc;
	bc.x = (BucketIndex)floor(point->x);
	bc.y = (BucketIndex)floor(point->y);
	bc.z = (BucketIndex)floor(point->z);

	AlignedCoordinate ac = AlignedCoordinate::Aligned(&bc);

	BucketHashIndex hash_index = m_shared.BucketHash_Find(&bc);

	if (hash_index == TInvalid(BucketHashIndex))
	{
		TUnimplemented(); // We need to drop a bucket and try again.
	}

	BucketHeader* bucket_header = &m_shared.m_buckets_hash[hash_index];

	// Either it's an empty slot or it contains our bucket, otherwise we have a problem.
	TAssert(!bucket_header->Valid() || bucket_header->m_coordinates.Equals(&ac));

	if (!bucket_header->Valid())
	{
		// Either it doesn't exist or it needs to be loaded from disk.
		bucket_header->Initialize(&ac);
		bucket_header->m_file_mapping = LoadBucket(bucket_header);
	}
	else
		// It's already in memory.
		bucket_header->Touch();

	ServerArtist* server_artist = &g_server_data->m_server_artists[from_peer];

	if (server_artist->m_current_stroke.m_stroke_index == TInvalid(StrokeIndex))
	{
		*PushVert(bucket_header, PushStroke(bucket_header)) = *point;

		server_artist->m_current_stroke.m_bucket = bucket_header->m_coordinates.m_bucket;
		server_artist->m_current_stroke.m_stroke_index = bucket_header->m_num_strokes-1;
	}
	else
	{
		TAssert(bucket_header->m_num_verts < bucket_header->m_max_verts);

		StrokeInfo* stroke = &bucket_header->m_strokes[server_artist->m_current_stroke.m_stroke_index];

		StrokeCoordinate last_stroke = bucket_header->GetLastStroke();
		if (server_artist->m_current_stroke.Equals(&last_stroke))
		{
			// If the last stroke in the bucket is our current stroke then we can just append.
			*PushVert(bucket_header, stroke) = *point;
		}
		else
			TUnimplemented();
	}
}

void ServerBuckets::EndStroke(net_peer_t from_peer)
{
	ServerArtist* server_artist = &g_server_data->m_server_artists[from_peer];
	TAssert(server_artist->m_current_stroke.Valid());

	BucketHashIndex hash_index = m_shared.BucketHash_Find(&server_artist->m_current_stroke.m_bucket);

	TAssert(hash_index != TInvalid(BucketHashIndex));

	BucketHeader* bucket_header = &m_shared.m_buckets_hash[hash_index];

	// If it's not the right bucket we have a problem.
	TAssert(bucket_header->Valid() && bucket_header->m_coordinates.m_bucket.Equals(&server_artist->m_current_stroke.m_bucket));

	auto* stroke = &bucket_header->m_strokes[server_artist->m_current_stroke.m_stroke_index];

	// Remove the stroke if there's only one vert.
	if (!stroke->m_previous.Valid() && !stroke->m_next.Valid() && stroke->m_num_verts <= 1)
		// It's not as easy as just num_strokes-- since we may not be the last stroke anymore.
		// Don't want to rearrange stuff here, so we'll just kick it down the line for the
		// defragmenter to worry about.
		stroke->m_num_verts = 0;

	server_artist->m_current_stroke.Invalidate();
}

StrokeInfo* ServerBuckets::PushStroke(BucketHeader* bucket_header)
{
	TAssert(bucket_header->m_num_strokes < bucket_header->m_max_strokes);

	StrokeInfo* stroke = &bucket_header->m_strokes[bucket_header->m_num_strokes];
	stroke->Initialize(bucket_header->m_num_verts);

	bucket_header->m_num_strokes++;

	FileMappingIndex index = bucket_header->m_file_mapping;
	TAssert(index != TInvalid(FileMappingIndex));

	auto* file_bucket = m_file_mappings[index].m_header->GetBucketSections(&bucket_header->m_coordinates);
	file_bucket->m_num_strokes++;

	return stroke;
}

vec3* ServerBuckets::PushVert(BucketHeader* bucket_header, StrokeInfo* stroke)
{
	TAssert(bucket_header->m_num_verts < bucket_header->m_max_verts);

	vec3* vert = &bucket_header->m_verts[stroke->m_first_vertex + stroke->m_num_verts];

	stroke->m_num_verts++;

	FileMappingIndex index = bucket_header->m_file_mapping;
	TAssert(index != TInvalid(FileMappingIndex));

	auto* file_bucket = m_file_mappings[index].m_header->GetBucketSections(&bucket_header->m_coordinates);
	file_bucket->m_num_verts++;

	return vert;
}

FileMappingIndex ServerBuckets::FindMapping(AlignedCoordinate* aligned, FileMappingIndex* empty)
{
	FileMappingIndex emptyempty = TInvalid(FileMappingIndex);

	// Look through our file mappings for The One
	for (int k = 0; k < NUM_FILE_MAPPINGS; k++)
	{
		if (!m_file_mappings[k].Valid())
			emptyempty = (FileMappingIndex)k;
		else if (m_file_mappings[k].m_bc.Equals(&aligned->m_aligned))
		{
			// We found The One
			TUnimplemented();
			return (FileMappingIndex)k;
		}
	}

	if (empty)
		*empty = emptyempty;

	return TInvalid(FileMappingIndex);
}

FileMappingIndex ServerBuckets::LoadBucket(BucketHeader* bucket)
{
	FileMappingIndex empty;
	FileMappingIndex index = FindMapping(&bucket->m_coordinates, &empty);

	if (empty == TInvalid(FileMappingIndex) && index == TInvalid(FileMappingIndex))
	{
		// There are no empty slots and we couldn't find the right mapping.
		TUnimplemented(); // Kick something else out?
		return TInvalid(FileMappingIndex);
	}

	if (index == TInvalid(FileMappingIndex))
		index = empty;

	BucketCoordinate bc = bucket->m_coordinates.m_bucket;
	BucketCoordinate aligned = bucket->m_coordinates.m_aligned;

	FileMapping* file_mapping = &m_file_mappings[index];

	char filename[100];
	sprintf(filename, "strokes_%d_%d_%d.sav", aligned.x, aligned.y, aligned.z);

	MapFile(filename, &file_mapping->m_memory);

	file_mapping->m_header = (FileMapping::SaveFileHeader*)file_mapping->m_memory.m_memory;
	file_mapping->m_bc = aligned;

	if (file_mapping->m_memory.m_created)
		file_mapping->CreateSaveFileHeader();

	auto* pointers = file_mapping->m_header->GetBucketSections(&bucket->m_coordinates);

	if (pointers->m_strokes_section == TInvalid(uint32))
		file_mapping->AllocStrokes(sizeof(StrokeInfo) * 100, &bc); // Room for 100 stroke fragments

	if (pointers->m_verts_section == TInvalid(uint32))
		file_mapping->AllocVerts(sizeof(vec3) * 1000, &bc); // Room for 1000 stroke points

	auto* strokes_section = &file_mapping->m_header->m_sections[pointers->m_strokes_section];
	auto* verts_section = &file_mapping->m_header->m_sections[pointers->m_verts_section];

	uint32 strokes_start = strokes_section->m_start;
	uint32 verts_start = verts_section->m_start;

	bucket->m_strokes = (StrokeInfo*)((uint8*)file_mapping->m_memory.m_memory + strokes_start);
	bucket->m_verts = (vec3*)((uint8*)file_mapping->m_memory.m_memory + verts_start);

	bucket->m_num_strokes = pointers->m_num_strokes;
	bucket->m_num_verts = pointers->m_num_verts;
	bucket->m_max_strokes = strokes_section->m_length/sizeof(StrokeInfo);
	bucket->m_max_verts = verts_section->m_length/sizeof(vec3);

	TAssert(stb_mod_eucl((size_t)bucket->m_strokes, 64) == 0);
	TAssert(stb_mod_eucl((size_t)bucket->m_verts, 64) == 0);

	return index;
}

void ServerBuckets::FileMapping::CreateSaveFileHeader()
{
	memset(m_header, 0, sizeof(*m_header));

	for (int i = 0; i < FILE_BUCKET_WIDTH; i++)
	{
		for (int j = 0; j < FILE_BUCKET_WIDTH; j++)
		{
			for (int k = 0; k < FILE_BUCKET_WIDTH; k++)
			{
				m_header->m_buckets[i][j][k].m_strokes_section = TInvalid(uint32);
				m_header->m_buckets[i][j][k].m_verts_section = TInvalid(uint32);
			}
		}
	}
}

uint32 ServerBuckets::FileMapping::AllocStrokes(uint32 size, BucketCoordinate* bc)
{
	uint32 result = Alloc(size);

	auto* bucket_sections = &m_header->m_buckets[bc->x - m_bc.x][bc->y - m_bc.y][bc->z - m_bc.z];
	bucket_sections->m_strokes_section = result;

	return result;
}

uint32 ServerBuckets::FileMapping::AllocVerts(uint32 size, BucketCoordinate* bc)
{
	uint32 result = Alloc(size);

	auto* bucket_sections = &m_header->m_buckets[bc->x - m_bc.x][bc->y - m_bc.y][bc->z - m_bc.z];
	bucket_sections->m_verts_section = result;

	return result;
}

uint32 ServerBuckets::FileMapping::Alloc(uint32 size)
{
	// Round up to the next cache line.
	int remainder = stb_mod_eucl(size, 64);
	if (remainder)
		size = size - remainder + 64;

	if (!m_header->m_sections[0].m_length)
	{
		// This is the very first allocation.
		TAssert(size <= m_memory.m_memory_size);
		m_header->m_sections[0].m_length = size;
		m_header->m_sections[0].m_start = m_header_size;
		return 0;
	}

	for (int k = 0; k < FILE_BUCKET_WIDTH * FILE_BUCKET_WIDTH * FILE_BUCKET_WIDTH * 2; k++)
	{
		if (!m_header->m_sections[k].m_length)
		{
			// We've come to the end of the list with no openings in the middle, we can put it here.

			TAssert(k > 0); // Should never happen since it's handled before this loop.

			uint32 start = m_header->m_sections[k-1].m_start + m_header->m_sections[k-1].m_length;

			// Return memory aligned to a cache line.
			int remainder = stb_mod_eucl(start, 64);
			if (remainder)
				start = start - remainder + 64;

			TAssert(start <= m_memory.m_memory_size);
			uint32 bytes_remaining = (uint32)m_memory.m_memory_size - start;

			if (bytes_remaining < size)
			{
				// We need to expand the file.
				TUnimplemented();
			}

			TAssert(bytes_remaining >= size);
			m_header->m_sections[k].m_start = start;
			m_header->m_sections[k].m_length = size;
			return k;
		}
	}

	return 0;
}

