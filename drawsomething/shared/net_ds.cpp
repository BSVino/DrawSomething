#include "artist.h"
#include "net_ds.h"

#ifdef CLIENT_LIBRARY
#include "client/ds_client.h"
#include "client/net_client.h"
#else
#include "server/ds_server.h"
#include "server/net_host.h"
#endif

#define STRUCT_OFFSET(s, f) ((uint16)&(s.f) - (uint16)&(s))

#ifdef CLIENT_LIBRARY
#define g_library_data g_client_data
#else
#define g_library_data g_server_data
#endif

void net_register_replications()
{
	NetShared* net_interface = &g_library_data->m_host.m_shared;
	DSNetShared* ds_net = &g_library_data->m_net_shared;

	replicated_field_t current_field;

	Artist a;

	ds_net->m_replicated_artist = net_interface->Replicated_AddEntity();

	current_field = net_interface->Replicated_AddField(STRUCT_OFFSET(a, m_looking), sizeof(a.m_looking) * 2 / 3, FT_FLOAT); // Only send the first two floats, pitch and yaw. Roll never changes.
	net_interface->m_replicated_fields_table[current_field].m_control = true;

	current_field = net_interface->Replicated_AddField(STRUCT_OFFSET(a, m_position), sizeof(a.m_position), FT_FLOAT);
	net_interface->m_replicated_fields_table[current_field].m_control = true;

	TAssert(current_field <= MAX_REPLICATED_FIELDS);
}

#ifdef CLIENT_LIBRARY
void* NetClient::GetEntityMemory(replicated_entity_instance_t /*entity_instance_index*/, replicated_entity_t entity_table_index, uint16 entity_index)
{
	if (g_client_data->m_net_shared.m_replicated_artist == entity_table_index)
		return &g_client_data->m_artists[entity_index];

	TUnimplemented();
	return NULL;
}

void* NetClient::GetEntityReplicatedMemory(replicated_entity_instance_t /*entity_instance_index*/, replicated_entity_t entity_table_index, uint16 entity_index)
{
	if (g_client_data->m_net_shared.m_replicated_artist == entity_table_index)
	{
		if (entity_index == g_client_data->m_host.m_peer_index)
			return &g_client_data->m_local_artist_replicated;
		else
			return NULL;
	}

	TUnimplemented();
	return NULL;
}

net_peer_t NetClient::GetPeerIndex(replicated_entity_instance_t /*entity_instance_index*/, replicated_entity_t entity_table_index, uint16 entity_index)
{
	if (g_client_data->m_net_shared.m_replicated_artist == entity_table_index)
	{
		TAssert(((net_peer_t)entity_index) == entity_index);
		return (net_peer_t)entity_index;
	}

	return TInvalid(net_peer_t);
}

void NetClient::AddEntityFromServerCallback(replicated_entity_instance_t /*entity_instance_index*/, replicated_entity_t entity_table_index, uint16 entity_index)
{
	g_client_data->m_artists[entity_index].m_active = 1;

	if (g_client_data->m_net_shared.m_replicated_artist == entity_table_index && entity_index == g_client_data->m_host.m_peer_index)
		g_client_data->m_local_artist.m_local = &g_client_data->m_artists[entity_index];
}
#else
void NetHost::ClientConnectedCallback(net_peer_t peer_index)
{
	g_server_data->m_artists[peer_index].m_active = 1;

	replicated_entity_instance_t entity_added_index = AddReplicated(g_server_data->m_net_shared.m_replicated_artist, peer_index, &g_server_data->m_artists[peer_index], &g_server_data->m_artists_replicated[peer_index]);
	m_shared.m_replicated_entities[entity_added_index].m_peer_index = peer_index;
}

void NetHost::Packet_ReceiveCustom(net_peer_t from_peer, uint8* data, uint32 data_length)
{
	TCheck(data_length >= 0);
	if (!data_length)
		return;

	switch (data[0])
	{
	case 'S':
	{
		ServerArtist* artist = &g_server_data->m_server_artists[from_peer];
		artist->m_incoming_points_size = 0;
		break;
	}

	case 'P':
	{
		TAssert(data_length == sizeof(vec3)+1);
		TAssert(artist->m_incoming_points_size < INCOMING_POINTS_SIZE);
		if (artist->m_incoming_points_size < INCOMING_POINTS_SIZE)
		{
			ServerArtist* artist = &g_server_data->m_server_artists[from_peer];
			artist->m_incoming_points[artist->m_incoming_points_size] = *(vec3*)(data+1);
			artist->m_incoming_points_size++;
		}
		break;
	}

	default:
		TUnimplemented();
		break;
	}
}
#endif

void tinker_enet_free(void* memory)
{
	ENetMemory* memory_reserve = &g_library_data->m_enet_memory;

	if (memory == &memory_reserve->m_host)
		return;
	else if (memory == &memory_reserve->m_peers[0])
		return;
	else if (memory == &memory_reserve->m_channels)
		return;

	else if (memory >= &memory_reserve->m_outgoing_commands[0] && memory < &memory_reserve->m_outgoing_commands[MAX_OUTGOING_COMMANDS])
	{
		memory_reserve->m_memory_table.FreeOutgoing(memory);
		return;
	}
	else if (memory >= &memory_reserve->m_acknowledgements[0] && memory < &memory_reserve->m_acknowledgements[MAX_ACKNOWLEDGEMENTS])
	{
		memory_reserve->m_memory_table.FreeAcknowledgement(memory);
		return;
	}
	else if (memory >= &memory_reserve->m_packets[0] && memory < &memory_reserve->m_packets[MAX_PACKETS])
	{
		memory_reserve->m_memory_table.FreePacket(memory);
		return;
	}
	else if (memory >= &memory_reserve->m_data[0] && memory < &memory_reserve->m_data[DATA_LENGTH][MAX_DATA])
	{
		memory_reserve->m_memory_table.FreeData(memory);
		return;
	}

	TCheck(false);
	free(memory);
}

void* tinker_enet_malloc(size_t size)
{
	ENetMemory* memory_reserve = &g_library_data->m_enet_memory;

	if (size == sizeof(memory_reserve->m_host))
		return &memory_reserve->m_host;
	else if (size == sizeof(memory_reserve->m_peers))
		return &memory_reserve->m_peers[0];
	else if (size == sizeof(memory_reserve->m_channels))
		return &memory_reserve->m_channels;

	else if (size == sizeof(memory_reserve->m_outgoing_commands[0]))
		return memory_reserve->m_memory_table.AllocOutgoing();
	else if (size == sizeof(memory_reserve->m_acknowledgements[0]))
		return memory_reserve->m_memory_table.AllocAcknowledgement();
	else if (size == sizeof(memory_reserve->m_packets[0]))
		return memory_reserve->m_memory_table.AllocPacket();
	else if (size <= DATA_LENGTH)
		return memory_reserve->m_memory_table.AllocData();

	TCheck(false);
	return malloc(size);
}

void tinker_enet_no_memory()
{
	// That sucks.
	TAssert(false);
}

void* ENetMemoryTable::AllocOutgoing()
{
	TAssert(MAX_OUTGOING_COMMANDS <= sizeof(m_occupied_outgoing) * 8);

	return Alloc(MAX_OUTGOING_COMMANDS, sizeof(ENetOutgoingCommand), &m_occupied_outgoing, (uint8*)&g_library_data->m_enet_memory.m_outgoing_commands[0]);
}

void ENetMemoryTable::FreeOutgoing(void* memory)
{
	Free(memory, MAX_OUTGOING_COMMANDS, sizeof(ENetOutgoingCommand), &m_occupied_outgoing, (uint8*)&g_library_data->m_enet_memory.m_outgoing_commands[0]);
}

void* ENetMemoryTable::AllocAcknowledgement()
{
	TAssert(MAX_ACKNOWLEDGEMENTS <= sizeof(m_occupied_acknowledgements) * 8);

	return Alloc(MAX_ACKNOWLEDGEMENTS, sizeof(ENetAcknowledgement), &m_occupied_acknowledgements, (uint8*)&g_library_data->m_enet_memory.m_acknowledgements[0]);
}

void ENetMemoryTable::FreeAcknowledgement(void* memory)
{
	Free(memory, MAX_ACKNOWLEDGEMENTS, sizeof(ENetAcknowledgement), &m_occupied_acknowledgements, (uint8*)&g_library_data->m_enet_memory.m_acknowledgements[0]);
}

void* ENetMemoryTable::AllocPacket()
{
	TAssert(MAX_PACKETS <= sizeof(m_occupied_packets) * 8);

	return Alloc(MAX_PACKETS, sizeof(ENetPacket), &m_occupied_packets, (uint8*)&g_library_data->m_enet_memory.m_packets[0]);
}

void ENetMemoryTable::FreePacket(void* memory)
{
	Free(memory, MAX_PACKETS, sizeof(ENetPacket), &m_occupied_packets, (uint8*)&g_library_data->m_enet_memory.m_packets[0]);
}

void* ENetMemoryTable::AllocData()
{
	TAssert(MAX_DATA <= DATA_LENGTH * 8);

	return Alloc(MAX_DATA, DATA_LENGTH, &m_occupied_data, (uint8*)&g_library_data->m_enet_memory.m_data[0][0]);
}

void ENetMemoryTable::FreeData(void* memory)
{
	Free(memory, MAX_DATA, DATA_LENGTH, &m_occupied_data, (uint8*)&g_library_data->m_enet_memory.m_data[0][0]);
}

void* ENetMemoryTable::Alloc(uint64 max, uint64 size, uint8* occupied, uint8* reserve)
{
	for (uint8 k = 0; k < max; k++)
	{
		if ((*occupied) & (1 << k))
			continue;

		(*occupied) |= (1 << k);
		return reserve + size * k;
	}

	TAssert(!"ENetMemoryTable::Alloc() - out of space");
	return 0;
}

void ENetMemoryTable::Free(void* memory, uint64 max, uint64 size, uint8* occupied, uint8* reserve)
{
	size_t first = (size_t)reserve;
	size_t outgoing = (size_t)memory;

	size_t offset = (outgoing - first) / size;

	TAssert((outgoing - first) % size == 0);
	TAssert(offset >= 0 && offset < max);

	(*occupied) &= ~(1 << offset);
}





