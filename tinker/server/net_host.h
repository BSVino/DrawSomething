#pragma once

#include "shared/net_shared.h"

struct NetHost
{
	struct _ENetHost*   m_enethost;

	net_peer_t m_max_players;

	NetShared m_shared;

	void Initialize();
	void Create(net_peer_t max_players);
	void Service();

	replicated_entity_instance_t AddReplicated(replicated_entity_t entity_table_index, uint16 entity_index, void* replicated_memory, void* replicated_memory_copy);

	// Internal stuff
	void ClientConnected(net_peer_t peer_index);

	// Game must implement these procedures
	void ClientConnectedCallback(net_peer_t peer_index);
};
