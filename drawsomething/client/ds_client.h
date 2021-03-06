#pragma once

#include "stringtable.h"

#include "client/renderer/shaders.h"
#include "client/client_buckets.h"
#include "net_ds.h"
#include "shared/tinker_shared.h"

#include "renderer/ds_renderer.h"
#include "game/c_artist.h"

#define VIEWBACK_MAIN_SPACE 1024
#define VIEWBACK_AF_SPACE 256
#define VIEWBACK_ITEM_SPACE 128

struct ClientData
{
	ClientBuckets m_buckets;

	ShaderLibrary m_shaders;
	DSRenderer    m_renderer;

	Artist m_artists[MAX_ARTISTS];
	Artist m_local_artist_replicated;
	LocalArtist m_local_artist;

	DSNetShared m_net_shared;
	ENetMemory m_enet_memory;

	TinkerShared m_shared;

	struct WindowData* m_window_data;

	// Viewback stuff.
	void* m_vb1;
	void* m_vb2;
	StringTable m_vb_strings;
	uint8 m_viewback_main[2][VIEWBACK_MAIN_SPACE];
	uint8 m_viewback_autofree[2][VIEWBACK_AF_SPACE];
	uint8 m_viewback_items[VIEWBACK_ITEM_SPACE];
	uint32 m_vb_items;
	uint8 m_vb_main1 : 1;
	uint8 m_vb_main2 : 1;
	uint8 m_vb_af1 : 1;
	uint8 m_vb_af2 : 1;

	ClientData(struct WindowData* window_data)
		: m_renderer(&m_shaders, window_data)
	{
		m_vb_items = m_vb_main1 = m_vb_main2 = m_vb_af1 = m_vb_af2 = 0;
	}

	void Initialize();

	Artist* GetLocalArtist();
};

extern ClientData* g_client_data;

// We reload the DLL so we have to store any strings we pass into VB.
inline const char* vb_str(const char* s)
{
	StringTableIndex i = st_find(g_client_data->m_vb_strings, s);

	size_t st_size = g_client_data->m_vb_strings.capacity();

	if (TInvalid(StringTableIndex) == i)
		i = st_add(g_client_data->m_vb_strings, pstring(s));

	// If it had to re-allocate then VB won't be able to find the strings.
	TAssert(g_client_data->m_vb_strings.capacity() == st_size);

	return st_get(g_client_data->m_vb_strings, i);
}

