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
	ENetMemory m_enet_memory;

	double m_game_time;
	float m_frame_time;
};

extern ServerData* g_server_data;

