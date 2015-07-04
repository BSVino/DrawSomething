#pragma once

#include "shared/tinker_shared.h"

#include "game/s_artist.h"

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

	ServerBuckets m_buckets;

	DSNetShared m_net_shared;
	ENetMemory m_enet_memory;

	TinkerShared m_shared;

	void Initialize();
};

extern ServerData* g_server_data;

