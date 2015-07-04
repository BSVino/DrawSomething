#pragma once

#ifdef CLIENT_LIBRARY
#include "client/net_client.h"
#else
#include "server/net_host.h"
#endif

#ifdef CLIENT_LIBRARY
#define g_game_data g_client_data
#else
#define g_game_data g_server_data
#endif

struct TinkerShared
{
	double m_game_time;
	double m_real_time;
	float m_frame_time;	

#ifdef CLIENT_LIBRARY
	NetClient m_host;
#else
	NetHost m_host;
#endif
};

extern TinkerShared* g_shared_data;
