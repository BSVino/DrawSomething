#include "client_buckets.h"

#include "ds_client.h"

void ClientBuckets::AddNewStroke()
{
	g_client_data->m_local_artist.m_current_stroke.Invalidate();
}

void ClientBuckets::AddPointToStroke(vec3* point)
{
	BucketCoordinate bc;
	bc.x = (BucketIndex)floor(point->x);
	bc.y = (BucketIndex)floor(point->y);
	bc.z = (BucketIndex)floor(point->z);

}


void ClientBuckets::EndStroke()
{

}
