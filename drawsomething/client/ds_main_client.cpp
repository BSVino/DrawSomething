#include <GL3/gl3w.h>

#include "tinker_platform.h"
#include "shell.h"

#include "gamecode.h"

#include "ds_client.h"

ClientData* g_client_data;

extern "C" TDLLEXPORT void GameLoad()
{
#if defined(__gl3w_h_)
	gl3wInit(); // We don't care about the return value
#endif
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

	g_client_data = new(game_data->m_memory) ClientData(game_data->m_window_data);

	GLint samples;
	glGetIntegerv(GL_SAMPLES, &samples);
	g_client_data->m_shaders.Initialize(samples);

	return 1;
}

extern "C" TDLLEXPORT bool GameFrame(GameData* game_data)
{
	g_client_data = (ClientData*)game_data->m_memory;

	g_client_data->m_renderer.Draw();

	return 1;
}
