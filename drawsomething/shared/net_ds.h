#pragma once

#include <enet/enet.h>

#ifdef CLIENT_LIBRARY
#include "client/net_client.h"
#else
#include "server/net_host.h"
#endif

#include "artist.h"

struct DSNetShared
{
	replicated_entity_t m_replicated_artist;
};

#define MAX_OUTGOING_COMMANDS 8
#define MAX_ACKNOWLEDGEMENTS 8
#define MAX_PACKETS 8
#define DATA_LENGTH 32
#define MAX_DATA 8

struct ENetMemoryTable
{
	uint8 m_occupied_outgoing;
	uint8 m_occupied_acknowledgements;
	uint8 m_occupied_packets;
	uint8 m_occupied_data;

	ENetMemoryTable()
	{
		m_occupied_outgoing = 0;
		m_occupied_acknowledgements = 0;
		m_occupied_packets = 0;
		m_occupied_data = 0;
	}

	void* Alloc(uint64 max, uint64 size, uint8* occupied, uint8* reserve);
	void Free(void* memory, uint64 max, uint64 size, uint8* occupied, uint8* reserve);
	void* AllocOutgoing();
	void FreeOutgoing(void* outgoing);
	void* AllocAcknowledgement();
	void FreeAcknowledgement(void* ack);
	void* AllocPacket();
	void FreePacket(void* packet);
	void* AllocData();
	void FreeData(void* data);
};

struct ENetMemory
{
	ENetHost m_host;

#ifdef CLIENT_LIBRARY
	ENetPeer m_peers[1];
#else
	ENetPeer m_peers[MAX_ARTISTS];
#endif

	ENetChannel m_channels;

	ENetMemoryTable     m_memory_table;
	ENetOutgoingCommand m_outgoing_commands[MAX_OUTGOING_COMMANDS];
	ENetAcknowledgement m_acknowledgements[MAX_ACKNOWLEDGEMENTS];
	ENetPacket          m_packets[MAX_PACKETS];
	uint8               m_data[DATA_LENGTH][MAX_DATA];
};


void net_register_replications();


