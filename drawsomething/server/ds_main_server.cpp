#include "ds_server.h"

#include "tinker_platform.h"
#include "gamecode.h"
#include "shell.h"

ServerData* g_server_data;

extern "C" TDLLEXPORT size_t GetMemorySize()
{
	return sizeof(ServerData);
}

extern "C" TDLLEXPORT void LibraryLoaded()
{
}

extern "C" TDLLEXPORT bool GameInitialize(GameData* game_data, int argc, char** args)
{
	g_shell.Initialize(argc, args);

	TCheck(game_data->m_memory_size >= sizeof(ServerData));
	if (game_data->m_memory_size < sizeof(ServerData))
		return 0;

	g_server_data = new(game_data->m_memory) ServerData();

	g_server_data->m_host.Initialize();
	g_server_data->m_host.Create(MAX_ARTISTS);

	return 1;
}

extern "C" TDLLEXPORT bool GameFrame(GameData* game_data)
{
	TCheck(game_data->m_memory_size >= sizeof(ServerData));
	if (game_data->m_memory_size < sizeof(ServerData))
		return 0;

	g_server_data = (ServerData*)game_data->m_memory;

	g_server_data->m_host.Service();

	return 1;
}
