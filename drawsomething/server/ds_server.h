#pragma once

#include "artist.h"

#include "server/net_host.h"

struct ServerData
{
	Artist m_artists[MAX_ARTISTS];

	NetHost m_host;
};

