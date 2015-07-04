#include "net_shared.h"

#include <string.h>
#include <math.h>

#include <enet/enet.h>

#include "tinker_shared.h"

struct PacketDelayHeader {
	double     m_delay_until_time;
	net_peer_t m_net_peer;
	int32      m_packet_size;
};

void NetShared::Initialize()
{
	m_replicated_fields_size = 0;
	m_replicated_entities_size = 0;
	m_replicated_fields_table_size = 0;
	m_replicated_entities_table_size = 0;

	TAssert(pow(2.0f, (float)sizeof(m_replicated_fields_size) * 8) > MAX_INSTANCE_FIELDS);
	TAssert(pow(2.0f, (float)sizeof(m_replicated_entities_size) * 8) > MAX_INSTANCE_ENTITIES);
	TAssert(pow(2.0f, (float)sizeof(m_replicated_fields_table_size) * 8) > MAX_REPLICATED_FIELDS);
	TAssert(pow(2.0f, (float)sizeof(m_replicated_entities_table_size) * 8) > MAX_REPLICATED_ENTITIES);
	TAssert(pow(2.0f, (float)sizeof(replicated_field_t) * 8) > MAX_REPLICATED_FIELDS);
	TAssert(pow(2.0f, (float)sizeof(replicated_entity_t) * 8) > MAX_REPLICATED_ENTITIES);
	TAssert(pow(2.0f, (float)sizeof(replicated_entity_instance_t) * 8) > MAX_INSTANCE_ENTITIES);
	TAssert(sizeof(replicated_field_t) == 1);           // If this gets larger, reads/writes need to start using tntoh/thton
	TAssert(sizeof(replicated_entity_t) == 1);          // If this gets larger, reads/writes need to start using tntoh/thton
	TAssert(sizeof(replicated_entity_instance_t) == 1); // If this gets larger, reads/writes need to start using tntoh/thton
	TAssert(pow(2.0f, (float)sizeof(net_peer_t) * 8) > MAX_PLAYERS);

#ifdef _DEBUG
	m_packet_send_delay = 0.8f;
#else
	m_packet_send_delay = 0;
#endif

	m_packet_queue_allocator.Initialize(m_packet_queue_memory, sizeof(m_packet_queue_memory));
}

void NetShared::Service()
{
	while (true)
	{
		PacketDelayHeader* header;
		int32 memory_size;
		m_packet_queue_allocator.PeekTail((void**)&header, &memory_size);
		if (!header)
			break;

		if (g_shared_data->m_real_time < header->m_delay_until_time)
			break;

		TAssert(memory_size - sizeof(PacketDelayHeader) == header->m_packet_size);

		g_shared_data->m_host.Packet_SendNow((uint8*)(header+1), header->m_packet_size, header->m_net_peer);

		m_packet_queue_allocator.FreeTail(nullptr, nullptr);
	}
}

replicated_entity_t NetShared::Replicated_AddEntity()
{
	TAssert(m_replicated_entities_table_size < MAX_REPLICATED_ENTITIES);
	m_replicated_entities_table[m_replicated_entities_table_size].m_field_table_start = m_replicated_entities_table_size;
	m_replicated_entities_table[m_replicated_entities_table_size].m_field_table_length = 0;
	return (replicated_entity_t)m_replicated_entities_table_size++;
}

replicated_field_t NetShared::Replicated_AddField(uint16 offset, uint8 size, field_type_t type)
{
	TAssert(m_replicated_fields_table_size < MAX_REPLICATED_FIELDS);
	m_replicated_fields_table[m_replicated_fields_table_size].m_offset = offset;
	m_replicated_fields_table[m_replicated_fields_table_size].m_size = size;
	m_replicated_fields_table[m_replicated_fields_table_size].m_type = type;

	m_replicated_entities_table[m_replicated_entities_table_size - 1].m_field_table_length++;

	return m_replicated_fields_table_size++;
}

void NetShared::Packet_WriteValueChange(uint8* packet_contents, uint16* packet_size, replicated_field_t table_entry_index, replicated_entity_instance_t entity_instance_index)
{
	ReplicatedField* table_entry = &m_replicated_fields_table[table_entry_index];
	ReplicatedInstanceEntity* entity_instance = &m_replicated_entities[entity_instance_index];

	packet_contents[1]++;

	uint16 current_size = *packet_size;

	uint8 item_size = table_entry->m_size; // The data we need to send
	item_size += 1; // The size of the data we need to send
	item_size += sizeof(replicated_field_t); // The ReplicatedField index
	item_size += sizeof(replicated_entity_instance_t); // The ReplicatedInstanceEntity index

	TAssert(item_size + current_size < MAX_PACKET_LENGTH);

	// Write the packet
	packet_contents[current_size] = table_entry->m_size;
	*(replicated_entity_instance_t*)(&packet_contents[current_size + 1]) = entity_instance_index;
	*(replicated_field_t*)(&packet_contents[current_size + 2]) = table_entry_index;

	uint8* dest = (uint8*)&packet_contents[current_size + 3];

	switch (table_entry->m_type)
	{
	case FT_INT8:
		memcpy(&packet_contents[current_size + 2], ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset), table_entry->m_size);
		break;

	case FT_INT16:
		TCheck(table_entry->m_size % 2 == 0);
		for (int j = 0; j < table_entry->m_size / 2; j++)
		{
			uint16* dest16 = ((uint16*)dest) + j;
			uint16* src = ((uint16*)ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset)) + j;
			*dest16 = thtons(*src);
		}
		break;

	case FT_INT32:
		TCheck(table_entry->m_size % 4 == 0);
		for (int j = 0; j < table_entry->m_size / 4; j++)
		{
			uint32* dest32 = ((uint32*)dest) + j;
			uint32* src = ((uint32*)ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset)) + j;
			*dest32 = thtonl(*src);
		}
		break;

	case FT_INT64:
		TCheck(table_entry->m_size % 8 == 0);
		for (int j = 0; j < table_entry->m_size / 8; j++)
		{
			uint64* dest64 = ((uint64*)dest) + j;
			uint64* src = ((uint64*)ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset)) + j;
			*dest64 = thtonll(*src);
		}
		break;

	case FT_FLOAT:
		TCheck(table_entry->m_size % 4 == 0);
		for (int j = 0; j < table_entry->m_size / 4; j++)
		{
			uint32* destf = ((uint32*)dest) + j;
			float* src = ((float*)ENTITY_FIELD_OFFSET(entity_instance->m_entity, table_entry->m_offset)) + j;
			*destf = thtonf(*src);
		}
		break;

	default:
		TUnimplemented();
		break;
	}

	*packet_size += item_size;
}

void NetShared::Packet_ReadValueChanges(uint8* packet, uint16 packet_size)
{
	TAssert(packet[0] == 'V');
	int values = packet[1];

	int current = 2;

	while (values)
	{
		uint8 entry_size = packet[current];
		replicated_entity_instance_t entity_instance_index = packet[current + 1];
		replicated_field_t table_entry_index = packet[current + 2];

		ReplicatedField* table_entry = &m_replicated_fields_table[table_entry_index];

		uint8* dest = (uint8*)ENTITY_FIELD_OFFSET(m_replicated_entities[entity_instance_index].m_entity, m_replicated_fields_table[table_entry_index].m_offset);
		void* src = (void*)&packet[current + 3];

		current += entry_size + 3;
		values--;

#ifndef CLIENT_LIBRARY
		// If I'm a server then I should only be receiving control values. Anything else is invalid and should be skipped.
		TCheck(table_entry->m_control);
		if (!table_entry->m_control)
			continue;
#endif

		switch (table_entry->m_type)
		{
		case FT_INT8:
			memcpy(dest, src, table_entry->m_size);
			break;

		case FT_INT16:
			TCheck(table_entry->m_size % 2 == 0);
			for (int j = 0; j < table_entry->m_size / 2; j++)
			{
				uint16* dest16 = ((uint16*)dest) + j;
				uint16* src16 = ((uint16*)src) + j;
				*dest16 = tntohs(*src16);
			}
			break;

		case FT_INT32:
			TCheck(table_entry->m_size % 4 == 0);
			for (int j = 0; j < table_entry->m_size / 4; j++)
			{
				uint32* dest32 = ((uint32*)dest) + j;
				uint32* src32 = ((uint32*)src) + j;
				*dest32 = tntohl(*src32);
			}
			break;

		case FT_INT64:
			TCheck(table_entry->m_size % 8 == 0);
			for (int j = 0; j < table_entry->m_size / 8; j++)
			{
				uint64* dest64 = ((uint64*)dest) + j;
				uint64* src64 = ((uint64*)src) + j;
				*dest64 = tntohll(*src64);
			}
			break;

		case FT_FLOAT:
			TCheck(table_entry->m_size % 4 == 0);
			for (int j = 0; j < table_entry->m_size / 4; j++)
			{
				float* destf = ((float*)dest) + j;
				float* srcf = ((float*)src) + j;
				*destf = tntohf(*(unsigned int*)srcf);
			}
			break;

		default:
			TUnimplemented();
			break;
		}
	}

	TAssert(current == packet_size);
}

void NetShared::Packet_Send(uint8* packet, uint16 packet_size, net_peer_t peer)
{
	if (m_packet_send_delay)
	{
		PacketDelayHeader* header = 0;
		while (!header)
		{
			header = (PacketDelayHeader*)m_packet_queue_allocator.Alloc(packet_size + sizeof(PacketDelayHeader));
			if (!header)
			{
				TAssert(false); // Make the ring buffer larger.
				
				if (m_packet_queue_allocator.IsEmpty())
				{
					g_shared_data->m_host.Packet_SendNow(packet, packet_size, peer);
					return;
				}

				PacketDelayHeader* header;
				int32 memory_size;
				m_packet_queue_allocator.FreeTail((void**)&header, &memory_size);
				g_shared_data->m_host.Packet_SendNow((uint8*)(header+1), header->m_packet_size, header->m_net_peer);
			}
		}

		header->m_delay_until_time = g_shared_data->m_real_time + m_packet_send_delay;
		header->m_net_peer = peer;
		header->m_packet_size = packet_size;
		memcpy((header+1), packet, packet_size);
	}
	else
	{
		while (!m_packet_queue_allocator.IsEmpty())
		{
			PacketDelayHeader* header;
			int32 memory_size;
			m_packet_queue_allocator.FreeTail((void**)&header, &memory_size);
			if (!header)
				break;

			TAssert(memory_size - sizeof(PacketDelayHeader) == header->m_packet_size);
		}

		// If you hit this, send everything in this list before continuing to ensure
		// everything is sent in the proper order.
		TAssert(m_packet_queue_allocator.IsEmpty());

		g_shared_data->m_host.Packet_SendNow(packet, packet_size, peer);
	}
}
