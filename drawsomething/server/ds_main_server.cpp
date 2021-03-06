#include "ds_server.h"

#include "tinker_platform.h"
#include "gamecode.h"
#include "shell.h"

#include "net_ds.h"

#include "../client/ds_client.h"

ServerData* g_server_data;
ClientData* g_client_data = nullptr; // This will only be non-null in listen servers!
TinkerShared* g_shared_data = nullptr;

extern "C" TDLLEXPORT size_t GetMemorySize()
{
	return sizeof(ServerData);
}

extern "C" TDLLEXPORT void LibraryLoaded()
{
	NetHost::LibraryLoad();
}

extern "C" TDLLEXPORT bool GameInitialize(GameData* game_data, int argc, char** args)
{
	g_shell.Initialize(argc, args);

	TCheck(game_data->m_memory_size >= sizeof(ServerData));
	if (game_data->m_memory_size < sizeof(ServerData))
		return 0;

	g_server_data = new(game_data->m_memory) ServerData();

	g_server_data->Initialize();

	return 1;
}

extern "C" TDLLEXPORT bool GameFrame(GameData* game_data)
{
	TCheck(game_data->m_memory_size >= sizeof(ServerData));
	if (game_data->m_memory_size < sizeof(ServerData))
		return 0;

	g_server_data = (ServerData*)game_data->m_memory;

	g_server_data->m_shared.m_game_time = game_data->m_game_time;
	g_server_data->m_shared.m_real_time = game_data->m_real_time;
	g_server_data->m_shared.m_frame_time = (float)game_data->m_frame_time;

	g_server_data->m_shared.m_host.Service();

	return 1;
}

extern "C" TDLLEXPORT void SetLocalNetworkMemory(GameData* game_data)
{
	g_client_data = (ClientData*)game_data->m_memory;
}

void ServerData::Initialize()
{
	g_shared_data = &m_shared;

	m_shared.m_host.Initialize();
	m_shared.m_host.Create(MAX_ARTISTS);
	net_register_replications();
}
