#include "net_host.h"

#include <enet/enet.h>

void NetHost::Initialize()
{
	enet_initialize();
}

void NetHost::Create(size_t max_players)
{
	ENetAddress address;
	address.host = ENET_HOST_ANY;
	address.port = 51072;

	m_enethost = enet_host_create(&address, max_players, 1, 0, 0);
}

void NetHost::Service()
{
	ENetEvent event;
	while (enet_host_service(m_enethost, &event, 0) > 0)
	{
		switch (event.type)
		{
		case ENET_EVENT_TYPE_CONNECT:
			//printf("A new client connected from %x:%u.\n",
			//	event.peer->address.host,
			//	event.peer->address.port);
			/* Store any relevant client information here. */
			event.peer->data = "Client information";
			break;
		case ENET_EVENT_TYPE_RECEIVE:
			//printf("A packet of length %u containing %s was received from %s on channel %u.\n",
			//	event.packet->dataLength,
			//	event.packet->data,
			//	event.peer->data,
			//	event.channelID);
			/* Clean up the packet now that we're done using it. */
			enet_packet_destroy(event.packet);
			break;
		case ENET_EVENT_TYPE_DISCONNECT:
			//printf("%s disconnected.\n", event.peer->data);
			/* Reset the peer's client information. */
			event.peer->data = NULL;
			break;
		}
	}
}

