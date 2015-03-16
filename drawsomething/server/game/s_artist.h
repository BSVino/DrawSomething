#pragma once

#include "artist.h"
#include "buckets.h"

// Properties of the artist that live only on the server.
struct ServerArtist
{
	Artist* m_local;

	StrokeCoordinate m_current_stroke;

	ServerArtist()
	{
		m_local = NULL;
	}
};
