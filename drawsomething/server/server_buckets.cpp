#include "buckets.h"

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
	TAssert(bucket_header->m_coordinates.x == TInvalid(BucketIndex) ||
		(bucket_header->m_coordinates.x == bc.x &&
		bucket_header->m_coordinates.y == bc.y &&
		bucket_header->m_coordinates.z == bc.z));

	if (bucket_header->m_coordinates.x == TInvalid(BucketIndex))
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

