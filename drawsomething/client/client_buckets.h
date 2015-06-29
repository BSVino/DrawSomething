#pragma once

#include "alloc_section.h"

#include "buckets.h"

#define NUM_CLIENT_BUCKETS 8 // If this grows above 2^8, increase BucketHashIndex

struct ClientBuckets
{
	SharedBuckets m_shared;

	// m_buckets_hash : BucketHashIndex -> BucketHeader
	BucketHeader m_buckets_hash[NUM_CLIENT_BUCKETS];

	struct BucketSections
	{
		int32 m_strokes_section; // index into m_sections
		int32 m_verts_section;   // index into m_sections
	} m_bucket_sections[NUM_CLIENT_BUCKETS]; // BucketHashIndex -> m_sections

	MemorySection m_memory_sections[NUM_CLIENT_BUCKETS*2];
	MemorySectionInfo m_memory_info;
	uint8 m_bucket_memory[1024*1024];

	SectionAllocator m_allocator;

	ClientBuckets()
		: m_shared(m_buckets_hash, NUM_CLIENT_BUCKETS)
	{}

	void Initialize();

	// Look up the bucket header by its coordinate.
	// If it's not present on the client, request it from the server,
	// but make sure its memory is valid in the meantime.
	BucketHeader* RetrieveBucket(BucketCoordinate* bc);
	void RequestBucket(BucketHeader* header);

	void AddNewStroke();
	void AddPointToStroke(vec3* new_point);
	void EndStroke();

	StrokeInfo* PushStroke(BucketHeader* bucket_header);
	vec3* PushVert(BucketHeader* bucket_header, StrokeIndex stroke);

	void ExpandStrokes(BucketHeader* bucket);
	void ExpandVerts(BucketHeader* bucket);
};

