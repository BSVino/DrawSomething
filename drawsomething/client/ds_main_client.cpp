#include <viewback2.h>

#include "tinker_gl.h"
#include "tinker_platform.h"
#include "shell.h"

#include "gamecode.h"
#include "input.h"
#include "window.h"

#include "ds_client.h"
#include "net_ds.h"

#include "../server/ds_server.h"

ClientData* g_client_data;

extern "C" TDLLEXPORT size_t GetMemorySize()
{
	return sizeof(ClientData);
}

extern "C" TDLLEXPORT void LibraryLoaded()
{
#if defined(__gl3w_h_)
	gl3wInit(); // We don't care about the return value
#endif

	InitializeNetworking();

	NetClient::LibraryLoad();
}

void* tinker_vb_alloc(size_t memory_size, vb_alloc_type_t type);
void tinker_vb_free(void* memory, vb_alloc_type_t type);

void ReloadShaders()
{
	g_client_data->m_shaders.CompileShaders();
	if (g_client_data->m_shaders.m_compiled)
		vb_console_append("Shaders reloaded.\n");
	else
		vb_console_append("Shaders compile failed. See shaders.txt\n");
}

extern "C" TDLLEXPORT bool GameInitialize(GameData* game_data, int argc, char** args)
{
	g_shell.Initialize(argc, args);

#if defined(__gl3w_h_)
	GLenum err = gl3wInit();
	if (0 != err)
		return 0;
#endif

	TCheck(game_data->m_memory_size >= sizeof(ClientData));
	if (game_data->m_memory_size < sizeof(ClientData))
		return 0;

	game_data->m_window_data->m_cursor_visible = false;

	g_client_data = new(game_data->m_memory) ClientData(game_data->m_window_data);

	g_client_data->m_window_data = game_data->m_window_data;

	g_client_data->Initialize();

	g_client_data->m_vb_strings.reserve(1024 * 10);

	vb2_config_t config;
	vb_config_initialize(&config);

	config.alloc_callback = tinker_vb_alloc;
	config.free_callback = tinker_vb_free;
	config.num_data_controls = 1;

	if (!vb_config_install(&config, 0, 0))
		return 0;

	vb_data_add_profile(vb_str("Default"), NULL);

	vb_data_add_control_button(vb_str("shaders_reload"), ReloadShaders);

	vb_server_create();

	vb_static_retrieve(&g_client_data->m_vb1, &g_client_data->m_vb2);

	g_client_data->m_host.Initialize();
	g_client_data->m_host.Connect("localhost");
	net_register_replications();

	return 1;
}

extern "C" TDLLEXPORT bool GameFrame(GameData* game_data)
{
	g_client_data = (ClientData*)game_data->m_memory;

	g_client_data->m_game_time = game_data->m_game_time;
	g_client_data->m_frame_time = (float)game_data->m_frame_time;

	vb_static_reset(g_client_data->m_vb1, g_client_data->m_vb2);

	vb_server_update(game_data->m_game_time);

	Artist* local = g_client_data->GetLocalArtist();

	if (local)
	{
		g_client_data->m_local_artist.HandleInput(game_data->m_input);

		vb_data_send_float_s(vb_str("pitch"), local->m_looking.p);
	}

	// This receives updates from the server, but it also sends commands, so we do it after we handle input.
	g_client_data->m_host.Service();

	g_client_data->m_local_artist.LocalThink();

	g_client_data->m_renderer.Draw();

	vb_static_retrieve(&g_client_data->m_vb1, &g_client_data->m_vb2);

	return 1;
}

ServerData* g_server_data = nullptr; // This will only be non-null in listen servers!

extern "C" TDLLEXPORT void SetLocalNetworkMemory(GameData* game_data)
{
	g_server_data = (ServerData*)game_data->m_memory;
}

void ClientData::Initialize()
{
	GLint samples;
	glGetIntegerv(GL_SAMPLES, &samples);

	m_shaders.Initialize(samples);

	m_renderer.base.Initialize();
	m_buckets.Initialize();
}

void* tinker_vb_alloc(size_t memory_size, vb_alloc_type_t type)
{
	switch (type)
	{
	case VB_AT_MAIN:
		TAssert(memory_size <= VIEWBACK_MAIN_SPACE);
		if (!g_client_data->m_vb_main1)
		{
			g_client_data->m_vb_main1 = 1;
			return &g_client_data->m_viewback_main[0];
		}
		else if (!g_client_data->m_vb_main2)
		{
			g_client_data->m_vb_main2 = 1;
			return &g_client_data->m_viewback_main[1];
		}

		TAssert(false);
		return 0;

	case VB_AT_AUTOFREE:
		TAssert(memory_size <= VIEWBACK_AF_SPACE);
		if (!g_client_data->m_vb_af1)
		{
			g_client_data->m_vb_af1 = 1;
			return &g_client_data->m_viewback_autofree[0];
		}
		else if (!g_client_data->m_vb_af2)
		{
			g_client_data->m_vb_af2 = 1;
			return &g_client_data->m_viewback_autofree[1];
		}

		TAssert(false);
		return 0;

	case VB_AT_AF_ITEM:
	{
		TAssert(g_client_data->m_vb_items + memory_size <= VIEWBACK_ITEM_SPACE);
		uint8* memory = &g_client_data->m_viewback_items[g_client_data->m_vb_items];
		g_client_data->m_vb_items += (uint32)memory_size;
		return memory;
	}

	default:
		TAssert(false);
		return 0;
	}
}

void tinker_vb_free(void* memory, vb_alloc_type_t type)
{
	switch (type)
	{
	case VB_AT_MAIN:
		if (memory == &g_client_data->m_viewback_main[0])
			g_client_data->m_vb_main1 = 0;
		else if (memory == &g_client_data->m_viewback_main[1])
			g_client_data->m_vb_main2 = 0;
		else
			TAssert(false);
		return;

	case VB_AT_AUTOFREE:
		if (memory == &g_client_data->m_viewback_autofree[0])
			g_client_data->m_vb_af1 = 0;
		else if (memory == &g_client_data->m_viewback_autofree[1])
			g_client_data->m_vb_af2 = 0;
		else
			TAssert(false);

		g_client_data->m_vb_items = 0;
		return;

	case VB_AT_AF_ITEM:
		// No op, item frees come before autofree free.
		return;

	default:
		TAssert(false);
		return;
	}
}
