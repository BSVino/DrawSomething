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

BucketHashIndex SharedBuckets::BucketHash_Insert(BucketCoordinate* coordinate)
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
			break;
	}

	return TInvalid(BucketHashIndex);
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

void BucketHeader::Initialize(BucketCoordinate* bc)
{
	m_coordinates.m_bucket = *bc;
	m_last_used_time = g_shared_data->m_game_time;
	m_num_strokes = m_num_verts = 0;
	m_max_strokes = m_max_verts = 0;
	m_strokes = 0;
	m_verts = 0;
}

void BucketHeader::Initialize(AlignedCoordinate* bc)
{
	m_coordinates = *bc;

	Initialize(&bc->m_bucket);
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

void BucketHeader::SetStrokeInfoMemory(void* stroke_info, int length)
{
	m_strokes = (StrokeInfo*)(stroke_info);
	m_max_strokes = length/sizeof(StrokeInfo);
}

void BucketHeader::SetVertsMemory(void* verts, int length)
{
	m_verts = (vec3*)(verts);
	m_max_verts = length/sizeof(vec3);
}

void BucketHeader::AddPointToStroke(vec3* point, StrokeCoordinate* stroke)
{
	BucketCoordinate bc;
	bc.x = (BucketIndex)floor(point->x);
	bc.y = (BucketIndex)floor(point->y);
	bc.z = (BucketIndex)floor(point->z);

	vec3 point_in_bucket_space = *point - vec3(bc.x, bc.y, bc.z);

	if (stroke->m_stroke_index == TInvalid(StrokeIndex))
	{
		StrokeInfo* new_stroke = g_shared_data->m_buckets.PushStroke(this);
		TAssert(new_stroke == &m_strokes[m_num_strokes-1]);
		*g_shared_data->m_buckets.PushVert(this, m_num_strokes-1) = point_in_bucket_space;

		stroke->m_bucket = m_coordinates.m_bucket;
		stroke->m_stroke_index = m_num_strokes-1;
	}
	else
	{
		StrokeCoordinate last_stroke = GetLastStroke();
		if (stroke->Equals(&last_stroke))
		{
			// If the last stroke in the bucket is our current stroke then we can just append.
			*g_shared_data->m_buckets.PushVert(this, stroke->m_stroke_index) = point_in_bucket_space;
		}
		else
		{
			// The last stroke in the current bucket is not the stroke we've
			// been working on.

			if (stroke->m_bucket.Equals(&bc))
			{
				// We're in the same bucket but we're not the most current stroke.
				// This can happen if another artist is drawing in the same bucket.
				TUnimplemented();
			}
			else
			{
				// The previous part of this stroke is in another bucket.
				BucketHeader* previous_bucket_header = g_shared_data->m_buckets.RetrieveBucket(&stroke->m_bucket);
				TAssert(Valid()); // Hopefully the old one wasn't pushed out.
				TAssert(previous_bucket_header->Valid());

				StrokeInfo* previous_stroke = &previous_bucket_header->m_strokes[stroke->m_stroke_index];
				TAssert(!previous_stroke->m_next.Valid());

				StrokeInfo* new_stroke = g_shared_data->m_buckets.PushStroke(this);
				StrokeIndex new_stroke_index = m_num_strokes-1;
				TAssert(new_stroke == &m_strokes[new_stroke_index]);

				*g_shared_data->m_buckets.PushVert(this, new_stroke_index) = point_in_bucket_space;

				previous_stroke->m_next.Set(&bc, &new_stroke_index);
				new_stroke->m_previous = *stroke;

				stroke->Set(&bc, &new_stroke_index);
			}
		}
	}
}

StrokeInfo* BucketHeader::PushStroke()
{
	TAssert(m_num_strokes < m_max_strokes);

	StrokeInfo* stroke = &m_strokes[m_num_strokes];
	stroke->Initialize(m_num_verts);

	m_num_strokes++;

	return stroke;
}

vec3* BucketHeader::PushVert(StrokeIndex stroke_index)
{
	TAssert(m_num_verts < m_max_verts);

	StrokeInfo* stroke = &m_strokes[stroke_index];
	vec3* vert = &m_verts[stroke->m_first_vertex + stroke->m_num_verts];

	stroke->m_num_verts++;
	m_num_verts++;

	return vert;
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
