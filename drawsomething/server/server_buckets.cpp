#include "buckets.h"

#include "stb_divide.h"
#include "tinker_platform.h"

#include "ds_server.h"
#include "game/s_artist.h"

void SharedBuckets::AddNewStroke(net_peer_t from_peer)
{
	ServerArtist* server_artist = &g_server_data->m_server_artists[from_peer];
	server_artist->m_current_stroke.m_stroke_index = TInvalid(StrokeIndex);
}

void SharedBuckets::AddPointToStroke(net_peer_t from_peer, vec3* point)
{
	BucketCoordinate bc;
	bc.x = (BucketIndex)point->x;
	bc.y = (BucketIndex)point->y;
	bc.z = (BucketIndex)point->z;

	BucketHashIndex hash_index = BucketHash_Find(&bc);

	TAssert(hash_index != TInvalid(BucketHashIndex));

	BucketHeader* bucket_header = &m_buckets_hash[hash_index];

	// Either it's an empty slot or it contains our bucket, otherwise we have a problem.
	TAssert(!bucket_header->Valid() ||
		(bucket_header->m_coordinates.x == bc.x &&
		bucket_header->m_coordinates.y == bc.y &&
		bucket_header->m_coordinates.z == bc.z));

	if (!bucket_header->Valid())
		bucket_header->Initialize(&bc);
	else
		bucket_header->Touch();

	ServerArtist* server_artist = &g_server_data->m_server_artists[from_peer];

	if (server_artist->m_current_stroke.m_stroke_index == TInvalid(StrokeIndex))
	{
		// Make a new stroke!
		StrokeInfo* stroke = &m_strokes[bucket_header->m_num_strokes];
		bucket_header->m_num_strokes++;

		stroke->Initialize(bucket_header->m_num_verts);

		bucket_header->m_verts[stroke->m_first_vertex + stroke->m_num_verts] = *point;
		stroke->m_num_verts++;
		bucket_header->m_num_verts++;
	}
	else
	{
		StrokeInfo* stroke = &m_strokes[server_artist->m_current_stroke.m_stroke_index];

		StrokeCoordinate last_stroke = bucket_header->GetLastStroke();
		if (server_artist->m_current_stroke.Equals(&last_stroke))
		{
			// If the last stroke in the bucket is our current stroke then we can just append.
			bucket_header->m_verts[stroke->m_first_vertex + stroke->m_num_verts] = *point;
			stroke->m_num_verts++;
			bucket_header->m_num_verts++;
		}
		else
			TUnimplemented();
	}
}

void SharedBuckets::EndStroke(net_peer_t from_peer)
{
	ServerArtist* server_artist = &g_server_data->m_server_artists[from_peer];
	TAssert(server_artist->m_current_stroke.Valid());

	// Remove the stroke if there's only one vert.
	TUnimplemented();
}

void* SharedBuckets::AllocateBucket(BucketCoordinate* bc)
{
	BucketCoordinate aligned_bc = *bc;

	aligned_bc.x -= stb_mod_eucl(bc->x, FILE_BUCKET_WIDTH);
	aligned_bc.y -= stb_mod_eucl(bc->y, FILE_BUCKET_WIDTH);
	aligned_bc.z -= stb_mod_eucl(bc->z, FILE_BUCKET_WIDTH);

	int empty = -1;
	// Look through our file mappings for The One
	for (int k = 0; k < NUM_FILE_MAPPINGS; k++)
	{
		if (!m_file_mappings[k].Valid())
			empty = k;

		else if (m_file_mappings[k].m_bc.Equals(&aligned_bc))
		{
			// We found The One
			TUnimplemented();
		}
	}

	// Not present. We need to allocate this memory.

	if (empty < 0)
	{
		TUnimplemented(); // Kick something else out?
		return NULL;
	}

	FileMapping* file_mapping = &m_file_mappings[empty];

	char filename[100];
	sprintf(filename, "strokes_%d_%d_%d.sav", aligned_bc.x, aligned_bc.y, aligned_bc.z);

	MapFile(filename, &file_mapping->m_memory);

	return file_mapping->m_memory.m_memory;
}

void BucketHeader::AllocateBucket(BucketCoordinate* bc)
{
	g_server_data->m_buckets.AllocateBucket(bc);
}
