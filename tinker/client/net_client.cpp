#include "net_client.h"

#include <common.h>

#include <enet/enet.h>

void NetClient::Initialize()
{
	enet_initialize();
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
	ENetEvent event;
	while (enet_host_service(m_enetclient, &event, 0) > 0)
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
		}
	}
}

