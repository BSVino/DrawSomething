#pragma once

#include "tinker_platform.h"
#include "vector.h"
#include "shared/net_shared.h"

typedef int32 BucketIndex;
typedef uint32 StrokeIndex;
typedef uint32 VertexIndex;
typedef uint8 BucketHashIndex;

// If this grows above 2^8, increase BucketHashIndex
#ifdef CLIENT_LIBRARY
#define NUM_BUCKETS 8
#else
#define NUM_BUCKETS 4
#define NUM_FILE_MAPPINGS NUM_BUCKETS
#define FILE_BUCKET_WIDTH 2 // How many buckets in per dimension go in a single file. 
#endif

#define NUM_STROKE_VERTS 100
#define NUM_STROKES 10

struct BucketCoordinate
{
	BucketIndex x, y, z;

	bool Equals(BucketCoordinate* other);

	BucketCoordinate Aligned();
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
	uint32      m_max_strokes;
	uint32      m_max_verts;

	void Initialize(BucketCoordinate* bc);
	void Invalidate();
	bool Valid();
	void Touch();

	StrokeCoordinate GetLastStroke();
};

struct SharedBuckets
{
	BucketHeader m_buckets_hash[NUM_BUCKETS];

	BucketHashIndex BucketHash_Find(BucketCoordinate* coordinate);

	SharedBuckets()
	{
		for (int k = 0; k < NUM_BUCKETS; k++)
			m_buckets_hash[k].Invalidate();
	}
};
