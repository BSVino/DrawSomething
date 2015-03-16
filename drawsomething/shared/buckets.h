#pragma once

#include "vector.h"
#include "shared/net_shared.h"

typedef uint32 BucketIndex;
typedef uint32 StrokeIndex;
typedef uint32 VertexIndex;
typedef uint8 BucketHashIndex;

struct BucketCoordinate
{
	BucketIndex x, y, z;

	bool Equals(BucketCoordinate* other);
};

struct StrokeCoordinate
{
	BucketCoordinate m_bucket;
	StrokeIndex m_stroke_index;

	bool Valid();
	bool Equals(StrokeCoordinate* other);
};

// In order to get the entire stroke you need to follow the previous and next
// pointers until you end up at a TInvalid(StrokeIndex).
struct StrokeInfo
{
	StrokeCoordinate m_previous; // Previous set in a continuous stroke
	StrokeCoordinate m_next;     // Next set in a continuous stroke
	VertexIndex m_first_vertex;
	uint32      m_num_verts;

	void Initialize(uint32 first_vertex);
};

struct BucketHeader
{
	BucketCoordinate m_coordinates;
	double      m_last_used_time;
	StrokeInfo* m_strokes;
	vec3*       m_verts;
	uint32      m_num_strokes;
	uint32      m_num_verts;

	void Initialize(BucketCoordinate* bc);
	void Touch();

	StrokeCoordinate GetLastStroke();
};

// If this grows above 2^8, increase BucketHashIndex
#ifdef CLIENT_LIBRARY
#define NUM_BUCKETS 8
#else
#define NUM_BUCKETS 4
#endif

#define NUM_STROKE_VERTS 100
#define NUM_STROKES 10

struct SharedBuckets
{
	BucketHeader m_buckets_hash[NUM_BUCKETS];

	vec3 m_stroke_verts[NUM_STROKE_VERTS];
	StrokeInfo m_strokes[NUM_STROKES];

	BucketHashIndex BucketHash_Find(BucketCoordinate* coordinate);

	SharedBuckets()
	{
		for (int k = 0; k < NUM_BUCKETS; k++)
			m_buckets_hash[k].m_coordinates.x = TInvalid(BucketIndex);
	}

#ifdef CLIENT_LIBRARY
#else
	void AddNewStroke(net_peer_t from_peer);
	void AddPointToStroke(net_peer_t from_peer, vec3* new_point);
	void EndStroke(net_peer_t from_peer);
#endif
};
