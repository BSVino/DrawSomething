#pragma once

#include "shared/net_shared.h"

struct NetClient
{
	struct _ENetPeer* m_enetpeer;
	struct _ENetHost * m_enetclient;

	NetShared m_shared;

	net_peer_t m_peer_index;

	void Initialize();
	void Connect(const char* connect);
	void Service();

	// Internal stuff
	void AddEntityFromServer(replicated_entity_instance_t entity_instance_index, replicated_entity_t entity_table_index, uint16 entity_index);

	// Game must implement these procedures
	net_peer_t GetPeerIndex(replicated_entity_instance_t entity_instance_index, replicated_entity_t entity_table_index, uint16 entity_index);
	void* GetEntityReplicatedMemory(replicated_entity_instance_t entity_instance_index, replicated_entity_t entity_table_index, uint16 entity_index);
	void* GetEntityMemory(replicated_entity_instance_t entity_instance_index, replicated_entity_t entity_table_index, uint16 entity_index);
};
