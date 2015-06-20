#pragma once

#include "tinker_platform.h"
#include "vector.h"
#include "shared/net_shared.h"

typedef int32 BucketIndex;
typedef uint32 StrokeIndex;
typedef uint32 VertexIndex;
typedef uint8 BucketHashIndex;
typedef uint8 FileMappingIndex;

#define NUM_STROKE_VERTS 100
#define NUM_STROKES 10

struct BucketCoordinate
{
	BucketIndex x, y, z;

	bool Equals(BucketCoordinate* other);

	BucketCoordinate Aligned();
};

struct AlignedCoordinate
{
	BucketCoordinate m_bucket;
	BucketCoordinate m_aligned;

	bool Equals(AlignedCoordinate* other);

	void operator = (AlignedCoordinate& o)
	{
		m_bucket = o.m_bucket;
		m_aligned = o.m_aligned;
	}

	static AlignedCoordinate Aligned(BucketCoordinate* bc);

	int GetXOffset()
	{
		return m_bucket.x - m_aligned.x;
	}

	int GetYOffset()
	{
		return m_bucket.y - m_aligned.y;
	}

	int GetZOffset()
	{
		return m_bucket.z - m_aligned.z;
	}
};

struct StrokeCoordinate
{
	BucketCoordinate m_bucket;
	StrokeIndex m_stroke_index;

	void Set(BucketCoordinate* bc, StrokeIndex* si);
	bool Valid();
	void Invalidate();
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
	AlignedCoordinate m_coordinates;
	double      m_last_used_time;

	// Never store a persistent pointer to an individual item in either of these two arrays.
	StrokeInfo* m_strokes; // Use PushStroke() to add a stroke
	vec3*       m_verts;   // Use PushVert() to add a vert

	uint32      m_num_strokes; // Current number of strokes
	uint32      m_num_verts;   // Current number of verts
	uint32      m_max_strokes; // Maximum strokes the buffer will hold
	uint32      m_max_verts;   // Maximum verts the buffer will hold

	FileMappingIndex m_file_mapping; // index into ServerBuckets::m_file_mappings

	void Initialize(AlignedCoordinate* bc);
	void Invalidate();
	bool Valid();
	void Touch();

	StrokeCoordinate GetLastStroke();
};

struct SharedBuckets
{
	// m_buckets_hash : BucketHashIndex -> BucketHeader
	BucketHeader* m_buckets_hash;
	int           m_num_buckets;

	BucketHashIndex BucketHash_Find(BucketCoordinate* coordinate);

	// Least recently used
	void GetLRUBucket(BucketHashIndex* LRU, double* LRU_time);

	SharedBuckets(BucketHeader* buckets_hash, int num_buckets)
	{
		m_buckets_hash = buckets_hash;
		m_num_buckets = num_buckets;

		for (int k = 0; k < num_buckets; k++)
			m_buckets_hash[k].Invalidate();
	}
};
