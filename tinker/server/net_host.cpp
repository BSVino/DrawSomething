#include "net_host.h"

#include <math.h>
#include <enet/enet.h>

void NetHost::Initialize()
{
	enet_initialize();

	m_shared.Initialize();
}

void NetHost::Create(net_peer_t max_players)
{
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 51072;

	m_enethost = enet_host_create(&address, max_players, 1, 0, 0);

	TAssert(max_players <= MAX_PLAYERS);
	m_max_players = max_players;
}

void NetHost::Service()
{
	uint16 packet_sizes[MAX_PLAYERS]; // How long in bytes is each packet?
	uint8 packet_contents[MAX_PLAYERS][MAX_PACKET_LENGTH];

	for (int k = 0; k < m_max_players; k++)
	{
		packet_sizes[k] = 2;

		packet_contents[k][0] = 'V';
		packet_contents[k][1] = 0; // How many values are in this packet?
	}

	for (int n = 0; n < m_shared.m_replicated_fields_size; n++)
	{
		replicated_field_t table_entry_index = m_shared.m_replicated_fields[n].m_table_entry;
		TAssert(table_entry_index < m_shared.m_replicated_fields_table_size);

		replicated_entity_instance_t entity_instance_index = m_shared.m_replicated_fields[n].m_instance_entry;

		ReplicatedField* table_entry = &m_shared.m_replicated_fields_table[table_entry_index];
		ReplicatedInstanceEntity* entity_instance = &m_shared.m_replicated_entities[entity_instance_index];

		if (memcmp(ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset), ENTITY_FIELD_OFFSET(entity_instance->m_entity_copy, table_entry->m_offset), table_entry->m_size) == 0)
			continue;

		for (int k = 0; k < m_max_players; k++)
		{
			if (table_entry->m_control && k == entity_instance->m_peer_index)
				continue;

			if (m_enethost->peers[k].state != ENET_PEER_STATE_CONNECTED)
				continue;

			m_shared.Packet_WriteValueChange(packet_contents[k], &packet_sizes[k], table_entry_index, entity_instance_index);
		}

		memcpy(ENTITY_FIELD_OFFSET(entity_instance->m_entity_copy, table_entry->m_offset), ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset), table_entry->m_size);
	}

	ENetPacket* packets[MAX_PLAYERS];
	for (int k = 0; k < m_max_players; k++)
	{
		if (!packet_contents[k][1])
			continue;

		packets[k] = enet_packet_create(packet_contents[k], packet_sizes[k], ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);
		int result = enet_peer_send(&m_enethost->peers[k], 0, packets[k]);
		// We allocated the packet contents on the stack, so we would have to
		// enet_host_flush if we weren't servicing immediately after.

		TCheck(result >= 0);
	}

	ENetEvent event;
	while (enet_host_service(m_enethost, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
		{
			net_peer_t peer_index = (net_peer_t)(event.peer - m_enethost->peers);
			ClientConnected(peer_index);
			break;
		}

		case ENET_EVENT_TYPE_RECEIVE:
			TCheck(event.packet->dataLength);
			if (!event.packet->dataLength)
				break;

			switch (event.packet->data[0])
			{
			case 'V':
				TAssert(((uint16)event.packet->dataLength) == event.packet->dataLength);
				m_shared.Packet_ReadValueChanges(event.packet->data, (uint16)event.packet->dataLength);
				break;

			default:
				TUnimplemented();
				break;
			}

			enet_packet_destroy(event.packet);
			break;

		case ENET_EVENT_TYPE_DISCONNECT:
			TUnimplemented();
			break;
		}
	}
}

replicated_entity_instance_t NetHost::AddReplicated(replicated_entity_t entity_table_index, uint16 entity_index, void* replicated_memory, void* replicated_memory_copy)
{
	TAssert(entity_table_index >= 0 && entity_table_index < m_shared.m_replicated_entities_table_size);
	TAssert(m_shared.m_replicated_fields_size + m_shared.m_replicated_entities_table[entity_table_index].m_field_table_length < MAX_INSTANCE_FIELDS);

	replicated_entity_instance_t entity_instance_index = m_shared.m_replicated_entities_size;
	m_shared.m_replicated_entities[entity_instance_index].m_entity = replicated_memory;
	m_shared.m_replicated_entities[entity_instance_index].m_entity_copy = replicated_memory_copy;
	m_shared.m_replicated_entities[entity_instance_index].m_entity_table_index = entity_table_index;
	m_shared.m_replicated_entities[entity_instance_index].m_entity_index = entity_index;
	m_shared.m_replicated_entities[entity_instance_index].m_peer_index = -1;
	m_shared.m_replicated_entities_size += 1;

	int max = m_shared.m_replicated_entities_table[entity_table_index].m_field_table_start + m_shared.m_replicated_entities_table[entity_table_index].m_field_table_length;
	for (int k = m_shared.m_replicated_entities_table[entity_table_index].m_field_table_start; k < max; k++)
	{
		memcpy(ENTITY_FIELD_OFFSET(replicated_memory_copy, m_shared.m_replicated_fields_table[k].m_offset), ENTITY_FIELD_OFFSET(replicated_memory, m_shared.m_replicated_fields_table[k].m_offset), m_shared.m_replicated_fields_table[k].m_size);

		m_shared.m_replicated_fields[m_shared.m_replicated_fields_size].m_table_entry = k;
		m_shared.m_replicated_fields[m_shared.m_replicated_fields_size].m_instance_entry = entity_instance_index;
		m_shared.m_replicated_fields_size += 1;
	}

	Packet_WriteCreateEntity(-1, entity_table_index, entity_index, entity_instance_index);

	return entity_instance_index;
}

void NetHost::ClientConnected(net_peer_t peer_index)
{
#ifdef _DEBUG
	enet_peer_timeout(&m_enethost->peers[peer_index], MAXUINT32, MAXUINT32, MAXUINT32);
#endif

	{
		const size_t packet_length = 2;
		uint8 packet_contents[packet_length];

		packet_contents[0] = 'W';
		packet_contents[1] = peer_index;

		ENetPacket* packet = enet_packet_create(packet_contents, packet_length, ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);
		enet_peer_send(&m_enethost->peers[peer_index], 0, packet);
		enet_host_flush(m_enethost); // Send packets now since packet_contents is about to go out of scope
	}

	{
		// Inform the new client of all existing ents
		for (int k = 0; k < m_shared.m_replicated_entities_size; k++)
			Packet_WriteCreateEntity(peer_index, m_shared.m_replicated_entities[k].m_entity_table_index, m_shared.m_replicated_entities[k].m_entity_index, k);

		uint16 packet_size; // In bytes
		uint8 packet_contents[MAX_PACKET_LENGTH];

		packet_size = 2;

		packet_contents[0] = 'V';
		packet_contents[1] = 0; // How many values are in this packet?

		for (int n = 0; n < m_shared.m_replicated_fields_size; n++)
		{
			replicated_field_t table_entry_index = m_shared.m_replicated_fields[n].m_table_entry;
			TAssert(table_entry_index < m_shared.m_replicated_fields_table_size);

			replicated_entity_instance_t entity_instance_index = m_shared.m_replicated_fields[n].m_instance_entry;

			ReplicatedField* table_entry = &m_shared.m_replicated_fields_table[table_entry_index];
			ReplicatedInstanceEntity* entity_instance = &m_shared.m_replicated_entities[entity_instance_index];

			m_shared.Packet_WriteValueChange(packet_contents, &packet_size, table_entry_index, entity_instance_index);
		}

		if (packet_contents[1])
		{
			ENetPacket* packet = enet_packet_create(packet_contents, packet_size, ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);
			int result = enet_peer_send(&m_enethost->peers[peer_index], 0, packet);
			enet_host_flush(m_enethost); // Send packets now since packet_contents is about to go out of scope

			TCheck(result >= 0);
		}
	}

	ClientConnectedCallback(peer_index);
}

void NetHost::Packet_WriteCreateEntity(net_peer_t destination_peer, replicated_entity_t entity_table_index, uint16 entity_index, replicated_entity_instance_t entity_instance_index)
{
	const size_t packet_length = 5;
	uint8 packet_contents[packet_length];

	packet_contents[0] = 'C';
	packet_contents[1] = entity_instance_index;
	packet_contents[2] = entity_table_index;
	*(uint16*)(&packet_contents[3]) = htons(entity_index);

	ENetPacket* packet = enet_packet_create(packet_contents, packet_length, ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);

	if (destination_peer == TInvalid(net_peer_t))
		enet_host_broadcast(m_enethost, 0, packet);
	else
	{
		TAssert(destination_peer < m_enethost->peerCount);
		enet_peer_send(&m_enethost->peers[destination_peer], 0, packet);
	}

	enet_host_flush(m_enethost); // Send packets now since packet_contents is about to go out of scope
}



