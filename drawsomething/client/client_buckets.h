#pragma once

#include "buckets.h"

#define NUM_CLIENT_BUCKETS 8 // If this grows above 2^8, increase BucketHashIndex

struct ClientBuckets
{
	SharedBuckets m_shared;

	// m_buckets_hash : BucketHashIndex -> BucketHeader
	BucketHeader m_buckets_hash[NUM_CLIENT_BUCKETS];

	ClientBuckets()
		: m_shared(m_buckets_hash, NUM_CLIENT_BUCKETS)
	{}

	void AddNewStroke();
	void AddPointToStroke(vec3* new_point);
	void EndStroke();
};

