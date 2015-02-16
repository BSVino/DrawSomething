#pragma once

#include "artist.h"

#include "server/net_host.h"
#include "net_ds.h"

struct ServerData
{
	Artist m_artists[MAX_ARTISTS];
	Artist m_artists_replicated[MAX_ARTISTS];

	NetHost m_host;
	DSNetShared m_net_shared;
};

extern ServerData* g_server_data;

