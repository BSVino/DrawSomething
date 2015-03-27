#pragma once

#include "stb_divide.h"

#include "buckets.h"

struct ServerBuckets
{
	SharedBuckets m_shared;

	struct FileMapping
	{
		BucketCoordinate m_bc;
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
				uint32 m_start;
				uint32 m_length;
			} m_sections[FILE_BUCKET_WIDTH * FILE_BUCKET_WIDTH * FILE_BUCKET_WIDTH * 2];
		}* m_header;

		uint32 m_header_size;

		FileMapping()
		{
			m_header = nullptr;

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
	} m_file_mappings[NUM_FILE_MAPPINGS];

	void AddNewStroke(net_peer_t from_peer);
	void AddPointToStroke(net_peer_t from_peer, vec3* new_point);
	void EndStroke(net_peer_t from_peer);

	void LoadBucket(BucketHeader* bucket);
};

