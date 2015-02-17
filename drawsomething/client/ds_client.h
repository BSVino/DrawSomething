#pragma once

#include "stringtable.h"

#include "client/renderer/shaders.h"
#include "client/net_client.h"
#include "net_ds.h"

#include "renderer/ds_renderer.h"
#include "artist.h"

struct ClientData
{
	ShaderLibrary m_shaders;
	DSRenderer    m_renderer;

	Artist m_artists[MAX_ARTISTS];
	Artist m_local_artist_replicated;

	NetClient m_client;
	DSNetShared m_net_shared;

	double m_game_time;
	float m_frame_time;

	// Viewback stuff.
	void* m_vb1;
	void* m_vb2;
	StringTable m_vb_strings;

	ClientData(struct WindowData* window_data)
		: m_renderer(&m_shaders, window_data)
	{
	}

	Artist* GetLocalArtist()
	{
		net_peer_t local = m_client.m_peer_index;
		if (local == TInvalid(net_peer_t))
			return NULL;

		return &m_artists[local];
	}
};

extern ClientData* g_client_data;

// We reload the DLL so we have to store any strings we pass into VB.
inline char* vb_str(char* s)
{
	StringTableIndex i = st_find(g_client_data->m_vb_strings, s);

	size_t st_size = g_client_data->m_vb_strings.capacity();

	if (TInvalid(StringTableIndex) == i)
		i = st_add(g_client_data->m_vb_strings, pstring(s));

	// If it had to re-allocate then VB won't be able to find the strings.
	TAssert(g_client_data->m_vb_strings.capacity() == st_size);

	return st_get(g_client_data->m_vb_strings, i);
}

