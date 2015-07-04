#include "alloc_ring.h"

void RingAllocator::Initialize(void* memory, int32 memory_size)
{
	m_memory = (uint8*)memory;
	m_memory_size = memory_size;
	m_head_index = m_tail_index = -1;
}

void* RingAllocator::Alloc(int32 size)
{
	if (m_head_index < 0)
	{
		// This is the first block allocated.
		if (sizeof(MemorySectionHeader) + size > m_memory_size)
		{
			TAssert(false);
			return 0;
		}

		m_head_index = m_tail_index = 0;

		MemorySectionHeader* header = (MemorySectionHeader*)m_memory;
		header->m_length = size;
		header->m_next = -1;

		return (void*)(header+1);
	}

	int limit = m_memory_size;
	if (m_head_index < m_tail_index)
		limit = m_tail_index;

	MemorySectionHeader* header = (MemorySectionHeader*)&m_memory[m_head_index];
	if (m_head_index + header->m_length + 2*sizeof(MemorySectionHeader) + size <= limit)
	{
		m_head_index += sizeof(MemorySectionHeader) + header->m_length;
		MemorySectionHeader* new_header = (MemorySectionHeader*)&m_memory[m_head_index];

		new_header->m_length = size;
		new_header->m_next = -1;

		header->m_next = m_head_index;

		return (void*)(new_header+1);
	}
	// Not enough room at the end. Is there enough room at the beginning?
	else if (m_head_index > m_tail_index && sizeof(MemorySectionHeader) + size <= m_tail_index)
	{
		m_head_index = 0;

		MemorySectionHeader* new_header = (MemorySectionHeader*)&m_memory[m_head_index];

		new_header->m_length = size;
		new_header->m_next = -1;

		header->m_next = m_head_index;

		return (void*)(new_header+1);
	}

	return 0;
}

void RingAllocator::PeekTail(void** start, int32* length)
{
	if (IsEmpty())
	{
		*start = nullptr;
		*length = 0;
		return;
	}

	TAssert(m_tail_index >= 0);

	uint8* memory = &m_memory[m_tail_index];
	MemorySectionHeader* header = (MemorySectionHeader*)memory;
	*length = header->m_length;
	*start = (void*)(header+1);
}

void RingAllocator::FreeTail(void** start, int32* length)
{
	if (IsEmpty())
	{
		TAssert(false);
		*start = nullptr;
		*length = 0;
		return;
	}

	TAssert(m_tail_index >= 0);

	MemorySectionHeader* header = (MemorySectionHeader*)&m_memory[m_tail_index];

	if (length)
		*length = header->m_length;

	if (start)
		*start = (void*)(header+1);

	if (header->m_next >= 0)
		m_tail_index = header->m_next;
	else
		m_head_index = m_tail_index = -1;
}

bool RingAllocator::IsEmpty()
{
	return m_head_index == -1;
}
