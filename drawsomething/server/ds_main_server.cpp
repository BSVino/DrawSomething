#include "tinker_platform.h"
#include "../tinker/gamecode.h"
#include "shell.h"

#include "player.h"

struct ServerData
{
	Player m_players[MAX_PLAYERS];
};

ServerData* g_server_data;

extern "C" TDLLEXPORT void GameLoad()
{

}

extern "C" TDLLEXPORT bool GameInitialize(GameData* game_data, int argc, char** args)
{
	g_shell.Initialize(argc, args);

	TAssert(game_data->m_memory_size >= sizeof(ServerData));
	if (game_data->m_memory_size < sizeof(ServerData))
		return 0;

	g_server_data = new(game_data->m_memory) ServerData();

	return 1;
}

extern "C" TDLLEXPORT bool GameFrame(GameData* game_data)
{
	TAssert(game_data->m_memory_size >= sizeof(ServerData));
	if (game_data->m_memory_size < sizeof(ServerData))
		return 0;

	g_server_data = (ServerData*)game_data->m_memory;

	return 1;
}
