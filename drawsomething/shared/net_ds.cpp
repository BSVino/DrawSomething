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

void net_register_replications()
{
#ifdef CLIENT_LIBRARY
	NetShared* net_interface = &g_client_data->m_client.m_shared;
	DSNetShared* ds_net = &g_client_data->m_net_shared;
#else
	NetShared* net_interface = &g_server_data->m_host.m_shared;
	DSNetShared* ds_net = &g_server_data->m_net_shared;
#endif

	replicated_field_t current_field;

	Artist a;

	ds_net->m_replicated_artist = net_interface->Replicated_AddEntity();
	current_field = net_interface->Replicated_AddField(STRUCT_OFFSET(a, m_looking), sizeof(a.m_looking) * 2 / 3, FT_FLOAT); // Only send the first two floats, pitch and yaw. Roll never changes.
	net_interface->m_replicated_fields_table[current_field].m_control = true;

	TAssert(current_field <= MAX_REPLICATED_FIELDS);
}

#ifdef CLIENT_LIBRARY
void* NetClient::GetEntityMemory(replicated_entity_instance_t entity_instance_index, replicated_entity_t entity_table_index, uint16 entity_index)
{
	if (g_client_data->m_net_shared.m_replicated_artist == entity_table_index)
		return &g_client_data->m_artists[entity_index];

	TUnimplemented();
	return NULL;
}

void* NetClient::GetEntityReplicatedMemory(replicated_entity_instance_t entity_instance_index, replicated_entity_t entity_table_index, uint16 entity_index)
{
	if (g_client_data->m_net_shared.m_replicated_artist == entity_table_index)
	{
		if (entity_index == g_client_data->m_client.m_peer_index)
			return &g_client_data->m_local_artist_replicated;
		else
			return NULL;
	}

	TUnimplemented();
	return NULL;
}

net_peer_t NetClient::GetPeerIndex(replicated_entity_instance_t entity_instance_index, replicated_entity_t entity_table_index, uint16 entity_index)
{
	if (g_client_data->m_net_shared.m_replicated_artist == entity_table_index)
	{
		TAssert(((net_peer_t)entity_index) == entity_index);
		return (net_peer_t)entity_index;
	}

	return TInvalid(net_peer_t);
}
#else
void NetHost::ClientConnectedCallback(net_peer_t peer_index)
{
	g_server_data->m_artists[peer_index].m_active = 1;

	replicated_entity_instance_t entity_added_index = AddReplicated(g_server_data->m_net_shared.m_replicated_artist, peer_index, &g_server_data->m_artists[peer_index], &g_server_data->m_artists_replicated[peer_index]);
	m_shared.m_replicated_entities[entity_added_index].m_peer_index = peer_index;
}
#endif

