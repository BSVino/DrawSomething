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

	BucketHeader* bucket_header = RetrieveBucket(&bc);

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
		{
			// The last stroke in the current bucket is not the stroke we've
			// been working on.

			if (server_artist->m_current_stroke.m_bucket.Equals(&bc))
			{
				// We're in the same bucket but we're not the most current stroke.
				// This can happen if another artist is drawing in the same bucket.
				TUnimplemented();
			}
			else
			{
				// The previous part of this stroke is in another bucket.
				BucketHeader* previous_bucket_header = RetrieveBucket(&server_artist->m_current_stroke.m_bucket);
				TAssert(bucket_header->Valid()); // Hopefully the old one wasn't pushed out.
				TAssert(previous_bucket_header->Valid());

				StrokeInfo* previous_stroke = &previous_bucket_header->m_strokes[server_artist->m_current_stroke.m_stroke_index];
				TAssert(!previous_stroke->m_next.Valid());

				StrokeInfo* new_stroke = PushStroke(bucket_header);
				StrokeIndex new_stroke_index = bucket_header->m_num_strokes-1;
				TAssert(new_stroke == &bucket_header->m_strokes[new_stroke_index]);

				*PushVert(bucket_header, new_stroke) = *point;

				previous_stroke->m_next.Set(&bc, &new_stroke_index);
				new_stroke->m_previous = server_artist->m_current_stroke;

				server_artist->m_current_stroke.Set(&bc, &new_stroke_index);
			}
		}
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
			// We found The One
			return (FileMappingIndex)k;
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
	file_mapping->m_num_active_buckets++;

	if (!file_mapping->Valid())
	{
		char filename[100];
		sprintf(filename, "strokes_%d_%d_%d.sav", aligned.x, aligned.y, aligned.z);

		MapFile(filename, &file_mapping->m_memory);

		file_mapping->m_header = (FileMapping::SaveFileHeader*)file_mapping->m_memory.m_memory;
		file_mapping->m_bc = aligned;

		if (file_mapping->m_memory.m_created)
			file_mapping->CreateSaveFileHeader();
	}

	// We are not caching the result of GetBucketSections because it invalidates over Alloc().
	if (file_mapping->m_header->GetBucketSections(&bucket->m_coordinates)->m_strokes_section == TInvalid(uint32))
		file_mapping->AllocStrokes(sizeof(StrokeInfo) * 100, &bc); // Room for 100 stroke fragments

	if (file_mapping->m_header->GetBucketSections(&bucket->m_coordinates)->m_verts_section == TInvalid(uint32))
		file_mapping->AllocVerts(sizeof(vec3) * 1000, &bc); // Room for 1000 stroke points

	auto* pointers = file_mapping->m_header->GetBucketSections(&bucket->m_coordinates);

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

BucketHeader* ServerBuckets::RetrieveBucket(BucketCoordinate* bc)
{
	AlignedCoordinate ac = AlignedCoordinate::Aligned(bc);

	BucketHashIndex hash_index = m_shared.BucketHash_Find(bc);

	if (hash_index == TInvalid(BucketHashIndex))
	{
		UnloadLRUBucket();
		hash_index = m_shared.BucketHash_Find(bc);
		TAssert(hash_index != TInvalid(BucketHashIndex));
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

	return bucket_header;
}

void ServerBuckets::UnloadLRUBucket()
{
	double LRU_time;
	BucketHashIndex LRU;

	m_shared.GetLRUBucket(&LRU, &LRU_time);

	UnloadBucket(LRU);
}

void ServerBuckets::UnloadBucket(BucketHashIndex i)
{
	TAssert(m_shared.m_buckets_hash[i].Valid());

	BucketHeader* header = &m_shared.m_buckets_hash[i];
	FileMappingIndex file_mapping_index = header->m_file_mapping;

	TAssert(file_mapping_index != TInvalid(FileMappingIndex));

	FileMapping* file_mapping = &m_file_mappings[file_mapping_index];

	file_mapping->m_num_active_buckets--;

	header->Invalidate();

	TAssert(!m_shared.m_buckets_hash[i].Valid());
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

	m_header->m_first_section = -1;

	for (int k = 0; k < TArraySize(m_header->m_sections); k++)
		m_header->m_sections[k].m_next = -1;
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

	if (m_header->m_first_section < 0)
	{
		// This is the very first allocation.

		int32 bytes_remaining = (int32)m_memory.m_memory_size - m_header_size;

		if (bytes_remaining < size)
		{
			// We need to expand the file
			ResizeMap(&m_memory, m_memory.m_memory_size * 2 + size);
			m_header = (FileMapping::SaveFileHeader*)m_memory.m_memory;

			bytes_remaining = (int32)m_memory.m_memory_size - m_header_size;
		}

		TAssert(bytes_remaining >= size);
		TAssert(size <= m_memory.m_memory_size);
		m_header->m_sections[0].m_length = size;
		m_header->m_sections[0].m_start = m_header_size;
		m_header->m_sections[0].m_next = -1;
		m_header->m_first_section = 0;
		return 0;
	}

	// Find an unused section to hold the info.
	int alloc_section = -1;
	for (int k = 0; k < TArraySize(m_header->m_sections); k++)
	{
		if (m_header->m_sections[k].m_length == 0)
		{
			alloc_section = k;
			break;
		}
	}

	TAssert(alloc_section >= 0);

	int prev_section = -1;
	int curr_section = m_header->m_first_section;

	if (m_header->m_sections[curr_section].m_start > m_header_size)
	{
		// We need to check the space before the first section.
		TUnimplemented();
	}

	while (true)
	{
		TAssert(curr_section >= 0);

		if (m_header->m_sections[curr_section].m_next < 0)
		{
			// This is the last one, we can allocate stuff after it.

			int32 start = m_header->m_sections[curr_section].m_start + m_header->m_sections[curr_section].m_length;

			TAssert(start <= m_memory.m_memory_size);

			// Return memory aligned to a cache line.
			int remainder = stb_mod_eucl(start, 64);
			if (remainder)
				start = start - remainder + 64;

			int32 bytes_remaining = (int32)m_memory.m_memory_size - start;

			if (bytes_remaining < size)
			{
				// We need to expand the file
				ResizeMap(&m_memory, m_memory.m_memory_size * 2 + size);
				m_header = (FileMapping::SaveFileHeader*)m_memory.m_memory;

				bytes_remaining = (int32)m_memory.m_memory_size - start;
			}

			TAssert(bytes_remaining >= size);
			m_header->m_sections[alloc_section].m_start = start;
			m_header->m_sections[alloc_section].m_length = size;
			m_header->m_sections[alloc_section].m_next = -1;
			m_header->m_sections[curr_section].m_next = alloc_section;
			return alloc_section;
		}
		else
		{
			int next_section = m_header->m_sections[curr_section].m_next;
			int32 next_start = m_header->m_sections[next_section].m_start;
			int32 this_start = m_header->m_sections[curr_section].m_start;
			int32 this_length = m_header->m_sections[curr_section].m_length;

			if (next_start - (this_start + this_length) > size)
			{
				TUnimplemented();
			}

			curr_section = next_section;
		}
	}

	return 0;
}
