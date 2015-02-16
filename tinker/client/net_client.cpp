#include "net_client.h"

#include <common.h>

#include <enet/enet.h>

void NetClient::Initialize()
{
	enet_initialize();

	m_shared.Initialize();
}

void NetClient::Connect(const char* connect)
{
	m_enetclient = enet_host_create(NULL, 1, 1, 350000, 95000); // This is 3 megabit d/l, .75 megabit upload. Should be the lowest common denominator of internet connection today.

	ENetAddress address;
	enet_address_set_host(&address, connect);
	address.port = 51072;

	m_enetpeer = enet_host_connect(m_enetclient, &address, 1, 0);

	ENetEvent event;
	if (enet_host_service(m_enetclient, &event, 5000) <= 0 || event.type != ENET_EVENT_TYPE_CONNECT)
	{
		TAssert(false);
		enet_peer_reset(m_enetpeer);
		return;
	}
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

		if (!table_entry->m_control)
			continue;

		if (memcmp(ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset), ENTITY_FIELD_OFFSET(entity_instance->m_entity_copy, table_entry->m_offset), table_entry->m_size) == 0)
			continue;

		m_shared.WriteValueChange(packet_contents, &packet_size, table_entry_index, entity_instance_index);

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
				TUnimplemented();
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

