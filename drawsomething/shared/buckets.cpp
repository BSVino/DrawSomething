#include "buckets.h"

#include "stb_divide.h"

#ifdef CLIENT_LIBRARY
#define g_shared_data g_client_data
#include "ds_client.h"
#else
#define g_shared_data g_server_data
#include "ds_server.h"
#endif

BucketHashIndex BucketHash_Hash(BucketCoordinate* coordinate)
{
	return (uint32)(((coordinate->x * 3985513591) + (coordinate->y * 2283571245) + (coordinate->z * 806576490))) % g_shared_data->m_buckets.m_shared.m_num_buckets;
}

BucketHashIndex SharedBuckets::BucketHash_Find(BucketCoordinate* coordinate)
{
	if (!coordinate)
		return TInvalid(BucketHashIndex);

	BucketHashIndex r;

	r = BucketHash_Hash(coordinate);
	BucketHashIndex first = r;
	TAssert(first < m_num_buckets);
	while (m_buckets_hash[r].Valid())
	{
		if (m_buckets_hash[r].m_coordinates.m_bucket.Equals(coordinate))
			return r;

		r = (r+1)%m_num_buckets;
		if (r == first)
			return TInvalid(BucketHashIndex);
	}

	return r;
}

void SharedBuckets::GetLRUBucket(BucketHashIndex* LRU, double* LRU_time)
{
	double lru_time = g_shared_data->m_game_time + 1;
	BucketHashIndex lru = TInvalid(BucketHashIndex);

	for (int k = 0; k < m_num_buckets; k++)
	{
		if (m_buckets_hash[k].m_last_used_time < lru_time)
		{
			lru = k;
			lru_time = m_buckets_hash[k].m_last_used_time;
		}
	}

	*LRU = lru;
	*LRU_time = lru_time;
}

bool BucketCoordinate::Equals(BucketCoordinate* other)
{
	return (x == other->x && y == other->y && z == other->z);
}

bool AlignedCoordinate::Equals(AlignedCoordinate* other)
{
	return m_bucket.Equals(&other->m_bucket);
}

void StrokeCoordinate::Set(BucketCoordinate* bc, StrokeIndex* si)
{
	m_bucket = *bc;
	m_stroke_index = *si;
}

bool StrokeCoordinate::Valid()
{
	return m_stroke_index != TInvalid(StrokeIndex);
}

void StrokeCoordinate::Invalidate()
{
	m_stroke_index = TInvalid(StrokeIndex);
}

bool StrokeCoordinate::Equals(StrokeCoordinate* other)
{
	return (m_bucket.Equals(&other->m_bucket) && m_stroke_index == other->m_stroke_index);
}

void StrokeInfo::Initialize(uint32 first_vertex)
{
	m_previous.m_stroke_index = TInvalid(StrokeIndex);
	m_next.m_stroke_index = TInvalid(StrokeIndex);
	m_first_vertex = first_vertex;
	m_num_verts = 0;
}

void BucketHeader::Initialize(AlignedCoordinate* bc)
{
	m_coordinates = *bc;
	m_last_used_time = g_shared_data->m_game_time;
	m_num_strokes = 0;
	m_num_verts = 0;
}

void BucketHeader::Invalidate()
{
	m_last_used_time = -1;
}

bool BucketHeader::Valid()
{
	return m_last_used_time >= 0;
}

void BucketHeader::Touch()
{
	m_last_used_time = g_shared_data->m_game_time;
}

StrokeCoordinate BucketHeader::GetLastStroke()
{
	StrokeCoordinate c;
	c.m_bucket = m_coordinates.m_bucket;
	c.m_stroke_index = TInvalid(StrokeIndex);

	if (!m_num_strokes)
		return c;

	c.m_stroke_index = m_num_strokes-1;

	return c;
}
