#include "tinker_platform.h"
#include "../tinker/gamecode.h"
#include "shell.h"

#include "artist.h"

struct ServerData
{
	Artist m_artists[MAX_ARTISTS];
};

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

	return 1;
}

extern "C" TDLLEXPORT bool GameFrame(GameData* game_data)
{
	TCheck(game_data->m_memory_size >= sizeof(ServerData));
	if (game_data->m_memory_size < sizeof(ServerData))
		return 0;

	g_server_data = (ServerData*)game_data->m_memory;

	return 1;
}
