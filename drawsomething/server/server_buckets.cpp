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

	bucket_header->AddPointToStroke(point, &server_artist->m_current_stroke);
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
	FileMappingIndex index = bucket_header->s.m_file_mapping;
	TAssert(index != TInvalid(FileMappingIndex));

	if (bucket_header->m_num_strokes >= bucket_header->m_max_strokes)
		m_file_mappings[index].ExpandStrokes(bucket_header);

	StrokeInfo* stroke = bucket_header->PushStroke();

	auto* file_bucket = m_file_mappings[index].m_header->GetBucketSections(&bucket_header->m_coordinates);
	file_bucket->m_num_strokes++;

	return stroke;
}

vec3* ServerBuckets::PushVert(BucketHeader* bucket_header, StrokeIndex stroke_index)
{
	FileMappingIndex index = bucket_header->s.m_file_mapping;
	TAssert(index != TInvalid(FileMappingIndex));

	if (bucket_header->m_num_verts >= bucket_header->m_max_verts)
		m_file_mappings[index].ExpandVerts(bucket_header);

	TAssert(bucket_header->m_num_verts < bucket_header->m_max_verts);

	return bucket_header->PushVert(stroke_index);
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

static int32 ResizeFileMapping(SectionAllocator* sa, int32 size)
{
	for (int k = 0; k < NUM_FILE_MAPPINGS; k++)
	{
		if (&g_server_data->m_buckets.m_file_mappings[k].m_allocator == sa)
		{
			g_server_data->m_buckets.m_file_mappings[k].ResizeMap(size);
			return g_server_data->m_buckets.m_file_mappings[k].m_memory.m_memory_size;
		}
	}

	TAssert(false); // bad
	return -1;
}

FileMappingIndex ServerBuckets::LoadBucket(BucketHeader* bucket)
{
	FileMappingIndex empty;
	FileMappingIndex index = FindMapping(&bucket->m_coordinates, &empty);

	if (empty == TInvalid(FileMappingIndex) && index == TInvalid(FileMappingIndex))
	{
		// There are no empty slots and we couldn't find the right mapping.
		UnloadInvactiveMappings();

		index = FindMapping(&bucket->m_coordinates, &empty);

		// Since we just freed one up, we should find a valid empty spot.
		TAssert(index == TInvalid(FileMappingIndex) && empty != TInvalid(FileMappingIndex));
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
		file_mapping->m_allocator.LoadFrom(file_mapping->m_header->m_sections, TArraySize(file_mapping->m_header->m_sections), file_mapping->m_memory.m_memory_size, &file_mapping->m_header->m_memory_info);
		file_mapping->m_allocator.SetResizeCallback(ResizeFileMapping);

		if (file_mapping->m_memory.m_created)
			file_mapping->CreateSaveFileHeader();
	}

	// We are not caching the result of GetBucketSections because it invalidates over Alloc().
	if (file_mapping->m_header->GetBucketSections(&bucket->m_coordinates)->m_strokes_section == TInvalid(uint32))
		file_mapping->AllocStrokes(sizeof(StrokeInfo) * 10, &bc); // Room for 100 stroke fragments

	if (file_mapping->m_header->GetBucketSections(&bucket->m_coordinates)->m_verts_section == TInvalid(uint32))
		file_mapping->AllocVerts(sizeof(vec3) * 1000, &bc); // Room for 1000 stroke points

	file_mapping->UpdateSectionPointers(bucket);

	auto* pointers = file_mapping->m_header->GetBucketSections(&bucket->m_coordinates);

	auto* strokes_section = &file_mapping->m_header->m_sections[pointers->m_strokes_section];
	auto* verts_section = &file_mapping->m_header->m_sections[pointers->m_verts_section];

	bucket->m_num_strokes = pointers->m_num_strokes;
	bucket->m_num_verts = pointers->m_num_verts;

	return index;
}

BucketHeader* ServerBuckets::RetrieveBucket(BucketCoordinate* bc)
{
	AlignedCoordinate ac = AlignedCoordinate::Aligned(bc);

	BucketHashIndex hash_index = m_shared.BucketHash_Insert(bc);

	if (hash_index == TInvalid(BucketHashIndex))
	{
		UnloadLRUBucket();
		hash_index = m_shared.BucketHash_Insert(bc);
		TAssert(hash_index != TInvalid(BucketHashIndex));
	}

	BucketHeader* bucket_header = &m_shared.m_buckets_hash[hash_index];

	// Either it's an empty slot or it contains our bucket, otherwise we have a problem.
	TAssert(!bucket_header->Valid() || bucket_header->m_coordinates.Equals(&ac));

	if (!bucket_header->Valid())
	{
		// Either it doesn't exist or it needs to be loaded from disk.
		bucket_header->Initialize(&ac);
		bucket_header->s.m_file_mapping = LoadBucket(bucket_header);
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

	TAssert(LRU != TInvalid(BucketHashIndex));
	TAssert(m_shared.m_buckets_hash[LRU].Valid());

	UnloadBucket(LRU);
}

void ServerBuckets::UnloadBucket(BucketHashIndex i)
{
	TAssert(m_shared.m_buckets_hash[i].Valid());

	BucketHeader* header = &m_shared.m_buckets_hash[i];
	FileMappingIndex file_mapping_index = header->s.m_file_mapping;

	TAssert(file_mapping_index != TInvalid(FileMappingIndex));

	FileMapping* file_mapping = &m_file_mappings[file_mapping_index];

	file_mapping->m_num_active_buckets--;

	auto* file_bucket = file_mapping->m_header->GetBucketSections(&header->m_coordinates);
	file_bucket->m_num_strokes = header->m_num_strokes;
	file_bucket->m_num_verts = header->m_num_verts;

	header->Invalidate();

	TAssert(!m_shared.m_buckets_hash[i].Valid());
}

void ServerBuckets::UnloadInvactiveMappings()
{
	FileMappingIndex available = TInvalid(FileMappingIndex);

	for (int k = 0; k < NUM_SERVER_BUCKETS; k++)
	{
		if (!m_file_mappings[k].m_num_active_buckets)
		{
			available = k;
			break;
		}
	}

	TAssert(available != TInvalid(FileMappingIndex));
	TAssert(m_file_mappings[available].Valid());

	UnmapFile(&m_file_mappings[available].m_memory);

	m_file_mappings[available].m_header = nullptr; // Invalidate
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

	m_allocator.Initialize(m_header->m_sections, TArraySize(m_header->m_sections), m_memory.m_memory_size, &m_header->m_memory_info);
	m_allocator.SetResizeCallback(ResizeFileMapping);
}

uint32 ServerBuckets::FileMapping::AllocStrokes(uint32 size, BucketCoordinate* bc)
{
	uint32 result = m_allocator.Alloc(size);

	auto* bucket_sections = &m_header->m_buckets[bc->x - m_bc.x][bc->y - m_bc.y][bc->z - m_bc.z];
	bucket_sections->m_strokes_section = result;

	return result;
}

uint32 ServerBuckets::FileMapping::AllocVerts(uint32 size, BucketCoordinate* bc)
{
	uint32 result = m_allocator.Alloc(size);

	auto* bucket_sections = &m_header->m_buckets[bc->x - m_bc.x][bc->y - m_bc.y][bc->z - m_bc.z];
	bucket_sections->m_verts_section = result;

	return result;
}

void ServerBuckets::FileMapping::ExpandStrokes(BucketHeader* bucket)
{
	auto* section = m_header->GetBucketSections(&bucket->m_coordinates);
	uint32 old_section = section->m_strokes_section;

	int new_size = m_header->m_sections[old_section].m_length * 2;

	int new_section = m_allocator.Alloc(new_size);

	// This pointer was invalidated by the alloc.
	section = m_header->GetBucketSections(&bucket->m_coordinates);
	section->m_strokes_section = new_section;

	memcpy((uint8*)m_memory.m_memory + m_header->m_sections[section->m_strokes_section].m_start + m_header_size,
		(uint8*)m_memory.m_memory + m_header->m_sections[old_section].m_start + m_header_size,
		m_header->m_sections[old_section].m_length);

	m_allocator.Free(old_section);

	UpdateSectionPointers(bucket);
}

void ServerBuckets::FileMapping::ExpandVerts(BucketHeader* bucket)
{
	auto* section = m_header->GetBucketSections(&bucket->m_coordinates);
	uint32 old_section = section->m_verts_section;

	int new_size = m_header->m_sections[old_section].m_length * 2;

	int new_section = m_allocator.Alloc(new_size);

	// This pointer was invalidated by the alloc.
	section = m_header->GetBucketSections(&bucket->m_coordinates);
	section->m_verts_section = new_section;

	memcpy((uint8*)m_memory.m_memory + m_header->m_sections[section->m_verts_section].m_start + m_header_size,
		(uint8*)m_memory.m_memory + m_header->m_sections[old_section].m_start + m_header_size,
		m_header->m_sections[old_section].m_length);

	m_allocator.Free(old_section);

	UpdateSectionPointers(bucket);
}

// If this proc has bad prof then it can be rewritten to iterate only buckets in
// this file mapping, but the file mapping would have to be modified to store that.
void ServerBuckets::FileMapping::ResizeMap(uint32 size)
{
	// We need to expand the file
	::ResizeMap(&m_memory, m_memory.m_memory_size * 2 + size);
	m_header = (FileMapping::SaveFileHeader*)m_memory.m_memory;

	m_allocator.LoadFrom(m_header->m_sections, TArraySize(m_header->m_sections), m_memory.m_memory_size, &m_header->m_memory_info);
	m_allocator.SetResizeCallback(ResizeFileMapping);

	// Remapping the memory invalidated stroke and vert pointers, now we should
	// update them.
	for (int k = 0; k < NUM_SERVER_BUCKETS; k++)
	{
		BucketHeader* bucket = &g_server_data->m_buckets.m_shared.m_buckets_hash[k];

		UpdateSectionPointers(bucket);
	}
}

void ServerBuckets::FileMapping::UpdateSectionPointers(BucketHeader* bucket)
{
	auto* pointers = m_header->GetBucketSections(&bucket->m_coordinates);

	if (pointers->m_strokes_section != TInvalid(uint32))
	{
		auto* strokes_section = &m_header->m_sections[pointers->m_strokes_section];
		uint32 strokes_start = strokes_section->m_start + m_header_size;
		bucket->SetStrokeInfoMemory((void*)((uint8*)m_memory.m_memory + strokes_start), strokes_section->m_length);
		TAssert(stb_mod_eucl((size_t)bucket->m_strokes, 64) == 0);
	}

	if (pointers->m_verts_section != TInvalid(uint32))
	{
		auto* verts_section = &m_header->m_sections[pointers->m_verts_section];
		uint32 verts_start = verts_section->m_start + m_header_size;
		bucket->SetVertsMemory((void*)((uint8*)m_memory.m_memory + verts_start), verts_section->m_length);
		TAssert(stb_mod_eucl((size_t)bucket->m_verts, 64) == 0);
	}
}

AlignedCoordinate AlignedCoordinate::Aligned(BucketCoordinate* bc)
{
	AlignedCoordinate aligned;
	aligned.m_bucket = *bc;
	aligned.m_aligned = bc->Aligned();
	return aligned;
}

BucketCoordinate BucketCoordinate::Aligned()
{
	BucketCoordinate aligned;
	aligned.x = x - stb_mod_eucl(x, FILE_BUCKET_WIDTH);
	aligned.y = y - stb_mod_eucl(y, FILE_BUCKET_WIDTH);
	aligned.z = z - stb_mod_eucl(z, FILE_BUCKET_WIDTH);
	return aligned;
}


