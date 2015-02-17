#include "net_client.h"

#include <common.h>

#include <enet/enet.h>

void NetClient::Initialize()
{
	enet_initialize();

	m_shared.Initialize();

	m_peer_index = TInvalid(net_peer_t);
}

void NetClient::Connect(const char* connect)
{
	m_enetclient = enet_host_create(NULL, 1, 1, 350000, 95000); // This is 3 megabit d/l, .75 megabit upload. Should be the lowest common denominator of internet connection today.

	ENetAddress address;
	enet_address_set_host(&address, connect);
	address.port = 51072;

	m_enetpeer = enet_host_connect(m_enetclient, &address, 1, 0);

#ifdef _DEBUG
	enet_peer_timeout(m_enetpeer, MAXUINT32, MAXUINT32, MAXUINT32);
#endif
}

void NetClient::Service()
{
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

		if (memcmp(ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset), ENTITY_FIELD_OFFSET(entity_instance->m_entity_copy, table_entry->m_offset), table_entry->m_size) == 0)
			continue;

		m_shared.Packet_WriteValueChange(packet_contents, &packet_size, table_entry_index, entity_instance_index);

		memcpy(ENTITY_FIELD_OFFSET(entity_instance->m_entity_copy, table_entry->m_offset), ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset), table_entry->m_size);
	}

	if (packet_contents[1])
	{
		ENetPacket* packet = enet_packet_create(packet_contents, packet_size, ENET_PACKET_FLAG_RELIABLE | ENET_PACKET_FLAG_NO_ALLOCATE);
		int result = enet_peer_send(m_enetpeer, 0, packet);

		TCheck(result >= 0);
	}

	ENetEvent event;
	while (enet_host_service(m_enetclient, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			break;

		case ENET_EVENT_TYPE_RECEIVE:
			TCheck(event.packet->dataLength);
			if (!event.packet->dataLength)
				break;

			switch (event.packet->data[0])
			{
			case 'C':
			{
				TAssert(event.packet->dataLength == 5);
				replicated_entity_instance_t entity_instance_index = event.packet->data[1];
				replicated_entity_t entity_table_index = event.packet->data[2];
				uint16 entity_index = ntohs(*(uint16*)&event.packet->data[3]);
				AddEntityFromServer(entity_instance_index, entity_table_index, entity_index);
				break;
			}

			case 'V':
				TAssert(((uint16)event.packet->dataLength) == event.packet->dataLength);
				m_shared.Packet_ReadValueChanges(event.packet->data, (uint16)event.packet->dataLength);
				break;

			case 'W':
				TAssert(event.packet->dataLength == 2);
				m_peer_index = event.packet->data[1];
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

void NetClient::AddEntityFromServer(replicated_entity_instance_t entity_instance_index, replicated_entity_t entity_table_index, uint16 entity_index)
{
	m_shared.m_replicated_entities[entity_instance_index].m_entity_table_index = entity_table_index;
	m_shared.m_replicated_entities[entity_instance_index].m_entity_index = entity_index;
	m_shared.m_replicated_entities[entity_instance_index].m_peer_index = GetPeerIndex(entity_instance_index, entity_table_index, entity_index);
	m_shared.m_replicated_entities[entity_instance_index].m_entity = GetEntityMemory(entity_instance_index, entity_table_index, entity_index);
	m_shared.m_replicated_entities[entity_instance_index].m_entity_copy = GetEntityReplicatedMemory(entity_instance_index, entity_table_index, entity_index);

	void* replicated_memory = m_shared.m_replicated_entities[entity_instance_index].m_entity;
	void* replicated_memory_copy = m_shared.m_replicated_entities[entity_instance_index].m_entity_copy;

	if (replicated_memory_copy)
	{
		int max = m_shared.m_replicated_entities_table[entity_table_index].m_field_table_start + m_shared.m_replicated_entities_table[entity_table_index].m_field_table_length;
		for (int k = m_shared.m_replicated_entities_table[entity_table_index].m_field_table_start; k < max; k++)
		{
			ReplicatedField* table_entry = &m_shared.m_replicated_fields_table[k];

			// We are only going to replicate controls to the server. Don't bother with anything else.
			if (!table_entry->m_control)
				continue;

			memcpy(ENTITY_FIELD_OFFSET(replicated_memory_copy, m_shared.m_replicated_fields_table[k].m_offset), ENTITY_FIELD_OFFSET(replicated_memory, m_shared.m_replicated_fields_table[k].m_offset), m_shared.m_replicated_fields_table[k].m_size);

			m_shared.m_replicated_fields[m_shared.m_replicated_fields_size].m_table_entry = k;
			m_shared.m_replicated_fields[m_shared.m_replicated_fields_size].m_instance_entry = entity_instance_index;
			m_shared.m_replicated_fields_size += 1;
		}
	}
}
