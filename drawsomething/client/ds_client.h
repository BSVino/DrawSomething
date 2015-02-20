#pragma once

#include "stringtable.h"

#include "client/renderer/shaders.h"
#include "client/net_client.h"
#include "net_ds.h"

#include "renderer/ds_renderer.h"
#include "game/c_artist.h"

struct ClientData
{
	ShaderLibrary m_shaders;
	DSRenderer    m_renderer;

	Artist m_artists[MAX_ARTISTS];
	Artist m_local_artist_replicated;
	LocalArtist m_local_artist;

	NetClient m_client_host;
	DSNetShared m_net_shared;

	struct WindowData* m_window_data;

	double m_game_time;
	float m_frame_time;

	// Temporary!
#define MAX_STROKE_POINTS 1000
	vec3 m_stroke_points[MAX_STROKE_POINTS];
#define MAX_STROKES 1000
	struct Stroke
	{
		// A stroke is m_stroke_points[first] to m_stroke_points[first + size - 1]
		int m_first;
		int m_size;
	} m_strokes[MAX_STROKES];
	int m_num_strokes;

	// Viewback stuff.
	void* m_vb1;
	void* m_vb2;
	StringTable m_vb_strings;

	ClientData(struct WindowData* window_data)
		: m_renderer(&m_shaders, window_data)
	{
		m_num_strokes = 0;
	}

	Artist* GetLocalArtist()
	{
		net_peer_t local = m_client_host.m_peer_index;
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

