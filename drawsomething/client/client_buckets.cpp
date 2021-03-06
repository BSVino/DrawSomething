#include "client_buckets.h"

#include "ds_client.h"

void ClientBuckets::Initialize()
{
	m_allocator.Initialize(m_memory_sections, TArraySize(m_memory_sections), TArraySize(m_bucket_memory));
	memset(m_bucket_sections, -1, sizeof(m_bucket_sections));
}

BucketHeader* ClientBuckets::RetrieveBucket(BucketCoordinate* bc)
{
	BucketHashIndex hash_index = m_shared.BucketHash_Insert(bc);

	if (hash_index == TInvalid(BucketHashIndex))
	{
		// TODO: unload FARTHEST bucket.
		double LRU_time;
		BucketHashIndex LRU;

		m_shared.GetLRUBucket(&LRU, &LRU_time);
		
		TAssert(m_shared.m_buckets_hash[LRU].Valid());
		m_shared.m_buckets_hash[LRU].Invalidate();

		if (m_bucket_sections[LRU].m_strokes_section >= 0)
			m_allocator.Free(m_bucket_sections[LRU].m_strokes_section);
		if (m_bucket_sections[LRU].m_verts_section >= 0)
			m_allocator.Free(m_bucket_sections[LRU].m_verts_section);

		m_bucket_sections[LRU].m_strokes_section = m_bucket_sections[LRU].m_verts_section = -1;

		hash_index = m_shared.BucketHash_Insert(bc);
		TAssert(hash_index != TInvalid(BucketHashIndex));
	}

	BucketHeader* bucket_header = &m_shared.m_buckets_hash[hash_index];

	// Either it's an empty slot or it contains our bucket, otherwise we have a problem.
	TAssert(!bucket_header->Valid() || bucket_header->m_coordinates.m_bucket.Equals(bc));

	if (!bucket_header->Valid())
	{
		// Either it doesn't exist or it needs to be loaded from the server.
		bucket_header->Initialize(bc);
		RequestBucket(bucket_header);
	}
	else
		// It's already in memory.
		bucket_header->Touch();

	return bucket_header;
}

void ClientBuckets::RequestBucket(BucketHeader* header)
{
	header->c.m_flags |= BucketHeader::HS_REQUESTED_FROM_SERVER;
}

void ClientBuckets::AddNewStroke()
{
	g_client_data->m_local_artist.m_current_stroke.Invalidate();
}

void ClientBuckets::AddPointToStroke(vec3* point)
{
	BucketCoordinate bc;
	bc.x = (BucketIndex)floor(point->x);
	bc.y = (BucketIndex)floor(point->y);
	bc.z = (BucketIndex)floor(point->z);

	BucketHeader* bucket_header = RetrieveBucket(&bc);

	LocalArtist* local_artist = &g_client_data->m_local_artist;

	bucket_header->AddPointToStroke(point, &local_artist->m_current_stroke);
}

void ClientBuckets::EndStroke()
{
	LocalArtist* local_artist = &g_client_data->m_local_artist;
	TAssert(local_artist->m_current_stroke.Valid());

	BucketHashIndex hash_index = m_shared.BucketHash_Find(&local_artist->m_current_stroke.m_bucket);

	TAssert(hash_index != TInvalid(BucketHashIndex));

	BucketHeader* bucket_header = &m_shared.m_buckets_hash[hash_index];

	// If it's not the right bucket we have a problem.
	TAssert(bucket_header->Valid() && bucket_header->m_coordinates.m_bucket.Equals(&local_artist->m_current_stroke.m_bucket));

	auto* stroke = &bucket_header->m_strokes[local_artist->m_current_stroke.m_stroke_index];

	// Remove the stroke if there's only one vert.
	if (!stroke->m_previous.Valid() && !stroke->m_next.Valid() && stroke->m_num_verts <= 1)
		// It's not as easy as just num_strokes-- since we may not be the last stroke anymore.
		// Don't want to rearrange stuff here, so we'll just kick it down the line for the
		// defragmenter to worry about.
		stroke->m_num_verts = 0;

	local_artist->m_current_stroke.Invalidate();
}

StrokeInfo* ClientBuckets::PushStroke(BucketHeader* bucket_header)
{
	if (bucket_header->m_num_strokes >= bucket_header->m_max_strokes)
		ExpandStrokes(bucket_header);

	return bucket_header->PushStroke();
}

vec3* ClientBuckets::PushVert(BucketHeader* bucket_header, StrokeIndex stroke_index)
{
	if (bucket_header->m_num_verts >= bucket_header->m_max_verts)
		ExpandVerts(bucket_header);

	return bucket_header->PushVert(stroke_index);
}

void ClientBuckets::ExpandStrokes(BucketHeader* bucket)
{
	BucketHashIndex bucket_index = m_shared.BucketHash_Find(&bucket->m_coordinates.m_bucket);

	int32 old_section = m_bucket_sections[bucket_index].m_strokes_section;

	int new_size = 1024;
	if (old_section >= 0)
		new_size = m_memory_sections[old_section].m_length * 2;

	int new_section = m_allocator.Alloc(new_size);

	TAssert(new_section >= 0);

	m_bucket_sections[bucket_index].m_strokes_section = new_section;

	if (old_section >= 0)
	{
		memcpy(m_bucket_memory + m_memory_sections[new_section].m_start,
			m_bucket_memory + m_memory_sections[old_section].m_start,
			m_memory_sections[old_section].m_length);

		m_allocator.Free(old_section);
	}

	m_buckets_hash[bucket_index].SetStrokeInfoMemory(m_bucket_memory + m_memory_sections[new_section].m_start, m_memory_sections[new_section].m_length);

	TAssert(bucket->m_num_strokes < bucket->m_max_strokes);
}

void ClientBuckets::ExpandVerts(BucketHeader* bucket)
{
	BucketHashIndex bucket_index = m_shared.BucketHash_Find(&bucket->m_coordinates.m_bucket);

	int32 old_section = m_bucket_sections[bucket_index].m_verts_section;

	int new_size = 1024;
	if (old_section >= 0)
		new_size = m_memory_sections[old_section].m_length * 2;

	int new_section = m_allocator.Alloc(new_size);

	TAssert(new_section >= 0);

	m_bucket_sections[bucket_index].m_verts_section = new_section;

	if (old_section >= 0)
	{
		memcpy(m_bucket_memory + m_memory_sections[new_section].m_start,
			m_bucket_memory + m_memory_sections[old_section].m_start,
			m_memory_sections[old_section].m_length);

		m_allocator.Free(old_section);
	}

	m_buckets_hash[bucket_index].SetVertsMemory(m_bucket_memory + m_memory_sections[new_section].m_start, m_memory_sections[new_section].m_length);

	TAssert(bucket->m_num_verts < bucket->m_max_verts);
}
