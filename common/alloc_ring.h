#pragma once

#include "common.h"

// Ring allocator. Implements a FIFO queue out of a block of memory.

struct MemorySectionHeader
{
	int32 m_length;
	int32 m_next; // Index into m_memory
};

struct RingAllocator
{
	uint8* m_memory;
	int32 m_memory_size;

	// m_head/tail_index are indexes into m_memory
	int32 m_head_index; // Head is the most recently alloc'd section
	int32 m_tail_index; // Tail is the section about to be freed

	// You allocate sections and pass it in. All datakeeping goes on in sections.
	// SectionAllocator never touches the actual memory. In fact you don't even
	// have to pass it in. memory_size is the size of the memory to be sectioned out.
	void Initialize(void* memory, int32 memory_size);

	// Request a section of memory.
	// If it returns 0, that means there was no space.
	void* Alloc(int32 size);
	void PeekTail(void** start, int32* length);
	void FreeTail(void** start, int32* length);
	bool IsEmpty();

	void CheckMemoryInfo();
};
