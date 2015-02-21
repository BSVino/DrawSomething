#include <GL3/gl3w.h>

#include <viewback2.h>

#include "tinker_platform.h"
#include "shell.h"

#include "gamecode.h"
#include "input.h"
#include "window.h"

#include "ds_client.h"
#include "net_ds.h"

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

	GLint samples;
	glGetIntegerv(GL_SAMPLES, &samples);
	g_client_data->m_shaders.Initialize(samples);
	g_client_data->m_renderer.base.Initialize();

	g_client_data->m_vb_strings.reserve(1024 * 10);

	vb2_config_t config;
	vb_config_initialize(&config);

	if (!vb_config_install(&config, 0, 0))
		return 0;

	vb_data_add_profile(vb_str("Default"), NULL);

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
