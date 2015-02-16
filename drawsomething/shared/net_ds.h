#pragma once

#ifdef CLIENT_LIBRARY
#include "client/net_client.h"
#else
#include "server/net_host.h"
#endif

struct DSNetShared
{
	replicated_entity_t m_replicated_artist;
};

void net_register_replications();


