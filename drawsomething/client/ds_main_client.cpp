#include "tinker_platform.h"
#include "../tinker/gamecode.h"
#include "shell.h"

#include <GL3/gl3w.h>

struct ClientData
{
};

ClientData* g_client_data;

extern "C" TDLLEXPORT bool GameInitialize(GameData* game_data, int argc, char** args)
{
	g_shell.Initialize(argc, args);

#if defined(__gl3w_h_)
	GLenum err = gl3wInit();
	if (0 != err)
		return 0;
#endif

	TAssert(game_data->m_memory_size >= sizeof(ClientData));
	if (game_data->m_memory_size < sizeof(ClientData))
		return 0;

	g_client_data = new(game_data->m_memory) ClientData();

	return 1;
}

extern "C" TDLLEXPORT bool GameFrame(GameData* game_data)
{
	g_client_data = (ClientData*)game_data->m_memory;

	return 1;
}
