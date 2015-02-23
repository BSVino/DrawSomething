#pragma once

#include "shared/net_shared.h"

struct NetHost
{
	struct _ENetHost*   m_enethost;

	net_peer_t m_max_players;

	NetShared m_shared;

	static void LibraryLoad();
	void Initialize();
	void Create(net_peer_t max_players);
	void Service();

	replicated_entity_instance_t AddReplicated(replicated_entity_t entity_table_index, uint16 entity_index, void* replicated_memory, void* replicated_memory_copy);

	// Internal stuff
	void ClientConnected(net_peer_t peer_index);

	// If destination_peer == TInvalid(net_peer_t) send to all peers
	void Packet_WriteCreateEntity(net_peer_t destination_peer, replicated_entity_t entity_table_index, uint16 entity_index, replicated_entity_instance_t entity_instance_index);

	// Game must implement these procedures
	void ClientConnectedCallback(net_peer_t peer_index);
};
