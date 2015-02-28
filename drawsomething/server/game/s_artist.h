#pragma once

#include "artist.h"

// Properties of the artist that live only on the server.
struct ServerArtist
{
	Artist* m_local;

#define INCOMING_POINTS_SIZE 1024
	vec3 m_incoming_points[INCOMING_POINTS_SIZE];
	uint32 m_incoming_points_size;

	ServerArtist()
	{
		m_local = NULL;
		m_incoming_points_size = 0;
	}
};
