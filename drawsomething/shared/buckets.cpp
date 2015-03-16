#include "buckets.h"

#ifdef CLIENT_LIBRARY
#define g_shared_data g_client_data
#else
#define g_shared_data g_server_data
#include "ds_server.h"
#endif

BucketHashIndex BucketHash_Hash(BucketCoordinate* coordinate)
{
	return ((coordinate->x * 3985513591) + (coordinate->y * 3985513592) + (coordinate->y * 3985513593))%NUM_BUCKETS;
}

BucketHashIndex SharedBuckets::BucketHash_Find(BucketCoordinate* coordinate)
{
	if (!coordinate)
		return TInvalid(BucketHashIndex);

	BucketHashIndex r;

	r = BucketHash_Hash(coordinate);
	BucketHashIndex first = r;
	while (m_buckets_hash[r].m_coordinates.x != TInvalid(BucketIndex))
	{
		r = (r+1)%NUM_BUCKETS;
		if (r == first)
		{
			TUnimplemented(); // Need to free up the least recently used bucket and make room for this one.
			break;
		}
	}

	return r;
}

bool BucketCoordinate::Equals(BucketCoordinate* other)
{
	return (x == other->x && y == other->y && z == other->z);
}

bool StrokeCoordinate::Valid()
{
	return m_stroke_index != TInvalid(StrokeIndex);
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

void BucketHeader::Initialize(BucketCoordinate* bc)
{
	m_coordinates = *bc;
	m_last_used_time = g_shared_data->m_game_time;
	m_num_strokes = 0;
	m_num_verts = 0;

	TUnimplemented(); // Allocate the memory
}

void BucketHeader::Touch()
{
	m_last_used_time = g_shared_data->m_game_time;
}

StrokeCoordinate BucketHeader::GetLastStroke()
{
	StrokeCoordinate c;
	c.m_bucket = m_coordinates;
	c.m_stroke_index = TInvalid(StrokeIndex);

	if (!m_num_strokes)
		return c;

	c.m_stroke_index = m_num_strokes-1;

	return c;
}