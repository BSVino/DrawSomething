#pragma once

#include "stb_divide.h"

#include "buckets.h"

struct ServerBuckets
{
	SharedBuckets m_shared;

	struct FileMapping
	{
		BucketCoordinate m_bc; // Aligned
		FileMappingInfo  m_memory;

		struct SaveFileHeader
		{
			struct BucketSections
			{
				uint32 m_strokes_section; // index into m_sections
				uint32 m_verts_section;   // index into m_sections
				uint32 m_num_strokes;
				uint32 m_num_verts;
			} m_buckets[FILE_BUCKET_WIDTH][FILE_BUCKET_WIDTH][FILE_BUCKET_WIDTH];

			struct MemorySection
			{
				int32 m_start;
				int32 m_length;
				int8  m_next;
			} m_sections[FILE_BUCKET_WIDTH * FILE_BUCKET_WIDTH * FILE_BUCKET_WIDTH * 2];

			int8 m_first_section;

			BucketSections* GetBucketSections(AlignedCoordinate* ac)
			{
				int x = ac->m_bucket.x - ac->m_aligned.x;
				int y = ac->m_bucket.y - ac->m_aligned.y;
				int z = ac->m_bucket.z - ac->m_aligned.z;

				return &m_buckets[x][y][z];
			}
		}* m_header;

		uint32 m_header_size; // TODO: Move this up to ServerBuckets to save memory
		uint8 m_num_active_buckets; // How many of the buckets in this file are using it?

		FileMapping()
		{
			m_header = nullptr;
			m_num_active_buckets = 0;

			m_header_size = sizeof(SaveFileHeader);

			// Round the header size up to the next cache line.
			int remainder = stb_mod_eucl(m_header_size, 64);
			if (remainder)
				m_header_size = m_header_size - remainder + 64;
		}

		bool Valid()
		{
			return !!m_header;
		}

		void CreateSaveFileHeader();

		uint32 AllocStrokes(uint32 size, BucketCoordinate* bc);
		uint32 AllocVerts(uint32 size, BucketCoordinate* bc);

		// Caller's responsibility to fix up m_buckets
		uint32 Alloc(uint32 size);
		void Free(uint32 section);

		void ResizeMap(uint32 size);

		void ExpandStrokes(BucketHeader* bucket);
		void ExpandVerts(BucketHeader* bucket);

		void UpdateSectionPointers(BucketHeader* bucket);
	} m_file_mappings[NUM_FILE_MAPPINGS];

	void AddNewStroke(net_peer_t from_peer);
	void AddPointToStroke(net_peer_t from_peer, vec3* new_point);
	void EndStroke(net_peer_t from_peer);

	StrokeInfo* PushStroke(BucketHeader* bucket_header);
	vec3* PushVert(BucketHeader* bucket_header, StrokeIndex stroke);

	FileMappingIndex FindMapping(AlignedCoordinate* bc, FileMappingIndex* empty = nullptr);
	FileMappingIndex LoadBucket(BucketHeader* bucket);

	BucketHeader* RetrieveBucket(BucketCoordinate* bc);

	void UnloadLRUBucket();
	void UnloadBucket(BucketHashIndex i);

	void UnloadInvactiveMappings();
};

