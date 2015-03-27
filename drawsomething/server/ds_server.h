#pragma once

#include "game/s_artist.h"

#include "server/net_host.h"
#include "net_ds.h"
#include "server_buckets.h"

struct ServerData
{
	// m_artists : net_peer_t -> Artist
	Artist m_artists[MAX_ARTISTS];
	// m_artists_replicated : net_peer_t -> Artist
	Artist m_artists_replicated[MAX_ARTISTS];

	// m_server_artists : net_peer_t -> ServerArtist
	ServerArtist m_server_artists[MAX_ARTISTS];

	NetHost m_host;
	DSNetShared m_net_shared;
	ENetMemory m_enet_memory;

	ServerBuckets m_buckets;

	double m_game_time;
	float m_frame_time;
};

extern ServerData* g_server_data;

