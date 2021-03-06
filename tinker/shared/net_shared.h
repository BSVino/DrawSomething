#pragma once

#include <common.h>

#include <alloc_ring.h>

typedef uint8 replicated_field_t;
typedef uint8 replicated_entity_t;
typedef uint8 replicated_entity_instance_t;
typedef uint8 net_peer_t;

#define MAX_INSTANCE_FIELDS 1024
#define MAX_INSTANCE_ENTITIES 128
#define MAX_REPLICATED_FIELDS 64
#define MAX_REPLICATED_ENTITIES 8
#define MAX_PLAYERS 64
#define MAX_PACKET_LENGTH 1024

#define ENTITY_FIELD_OFFSET(m, o) (void*)(((size_t)m) + (o))

// On the server: Describes a field that needs to be replicated to all clients if it changes.
// On the client: Describes a field that the server is replicating to us.
struct ReplicatedInstanceField
{
	replicated_field_t m_table_entry;
	replicated_entity_instance_t m_instance_entry;
};

// Describes an entity in memory whose fields we are tracking.
struct ReplicatedInstanceEntity
{
	void* m_entity;          // The live copy
	void* m_entity_copy;     // The duplicate for comparison to see if data has changed
	uint16 m_entity_index;   // Index of the entity, game-specified data
	replicated_entity_t m_entity_table_index; // Index into m_replicated_entities_table
	net_peer_t m_peer_index; // Non-negative if this entity is a client connected to the server, negative otherwise.
};

typedef enum
{
	FT_NONE = 0,
	FT_INT8,
	FT_INT16,
	FT_INT32,
	FT_INT64,
	FT_FLOAT,
} field_type_t;

// Describes a field of an object that gets replicated when that object is created.
struct ReplicatedField
{
	uint16 m_offset;
	uint8  m_size;
	field_type_t m_type;
	uint8  m_control : 1; // This is a control field for a player. It's changed by client code and sent down to the server, which replicates it to all other clients.
};

struct ReplicatedEntity
{
	replicated_field_t m_field_table_start;
	uint16 m_field_table_length;
};

struct NetShared
{
	ReplicatedInstanceField m_replicated_fields[MAX_INSTANCE_FIELDS];
	ReplicatedInstanceEntity m_replicated_entities[MAX_INSTANCE_ENTITIES]; // indexed by replicated_entity_instance_t
	ReplicatedField m_replicated_fields_table[MAX_REPLICATED_FIELDS];      // indexed by replicated_field_t
	ReplicatedEntity m_replicated_entities_table[MAX_REPLICATED_ENTITIES]; // indexed by replicated_entity_t

	uint16 m_replicated_fields_size;
	uint8 m_replicated_entities_size;
	uint8 m_replicated_fields_table_size;
	uint8 m_replicated_entities_table_size;

	float m_packet_send_delay;
	RingAllocator m_packet_queue_allocator;
	uint8 m_packet_queue_memory[4096];

	void Initialize();

	void Service();

	replicated_entity_t Replicated_AddEntity();
	replicated_field_t Replicated_AddField(uint16 offset, uint8 size, field_type_t type);

	// The header for the value changed packet should be already written here, WriteValueChange just appends an entry.
	void Packet_WriteValueChange(uint8* packet_contents, uint16* packet_size, replicated_field_t table_entry_index, replicated_entity_instance_t entity_instance_index);
	// Reads all of the value changed packet and updates fields and everything.
	void Packet_ReadValueChanges(uint8* packet, uint16 packet_size);

	// Put a packet on the list of packets to be sent.
	// This always copies the memory of packet elsewhere so it can be freed immediately.
	void Packet_Send(uint8* packet, uint16 packet_size, net_peer_t peer);
};

// The game must define these. They can be simple pass-throughs.
void tinker_enet_free(void*);
void* tinker_enet_malloc(size_t);
void tinker_enet_no_memory();

// All of my ship platforms are little endian so use my own ntof so
// I don't do all the work to convert all my data to and from big endian.
#if defined(_WIN32) || __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define thtons(x)  (x)
#define thtonl(x)  (x)
#define thtonll(x) (x)
#define thtonf(x)  (x)

#define tntohs(x)  (x)
#define tntohl(x)  (x)
#define tntohll(x) (x)
#define tntohf(x)  (x)
#else
#error !
#endif

/*
Player welcome message packet -- Sent to the connecting client when it first connects. Format:
	Byte 0: 'W'
	Byte 1: A unique peer index
2 bytes total.



Value changed packet format:
	Byte 0: 'V'
	Byte 1: The number of value changes in this packet

	for each value changed:
		Byte n+0: size in bytes of the value
		Byte n+1: entity network instance index for the entity being updated
		Byte n+2: field table entry index
		Byte n+3: updated value (size bytes, network byte order)



Entity created packet format:
	Byte 0: 'C'
	Byte 1: entity network instance index
	Byte 2: entity network table index
	Byte 3-4: entity index -- game defined
5 bytes total.
*/

