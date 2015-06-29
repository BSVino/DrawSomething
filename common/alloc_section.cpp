#include "alloc_section.h"

#include "stb_divide.h"

void SectionAllocator::Initialize(MemorySection* sections, int num_sections, int32 memory_size, MemorySectionInfo* memory_info)
{
	if (!memory_info)
		memory_info = &m_memory_info_storage;

	LoadFrom(sections, num_sections, memory_size, memory_info);

	m_memory_info->m_first_section = -1;
	for (int k = 0; k < num_sections; k++)
		m_sections[k].m_next = -1;
}

void SectionAllocator::LoadFrom(MemorySection* sections, int num_sections, int32 memory_size, MemorySectionInfo* memory_info)
{
	m_num_sections = num_sections;
	m_sections = sections;
	m_memory_size = memory_size;

	m_memory_info = memory_info;
}

int32 SectionAllocator::Alloc(int32 size)
{
	CheckMemoryInfo();

	// Round up to the next cache line.
	int remainder = stb_mod_eucl(size, 64);
	if (remainder)
		size = size - remainder + 64;

	if (m_memory_info->m_first_section < 0)
	{
		// This is the very first allocation.

		int32 bytes_remaining = m_memory_size;

		if (bytes_remaining < size)
		{
			if (!ResizeBackingMemory(size))
				return -1;

			bytes_remaining = (int32)m_memory_size;
		}

		TAssert(bytes_remaining >= size);
		TAssert(size <= m_memory_size);
		m_sections[0].m_length = size;
		m_sections[0].m_start = 0;
		m_sections[0].m_next = -1;
		m_memory_info->m_first_section = 0;
		CheckMemoryInfo();
		return 0;
	}

	// Find an unused section to hold the info.
	int alloc_section = -1;
	for (int k = 0; k < m_num_sections; k++)
	{
		if (m_sections[k].m_length == 0)
		{
			alloc_section = k;
			break;
		}
	}

	TAssert(alloc_section >= 0);

	int prev_section = -1;
	int curr_section = m_memory_info->m_first_section;

	if (m_sections[curr_section].m_start >= size)
	{
		m_sections[alloc_section].m_start = 0;
		m_sections[alloc_section].m_length = size;
		m_sections[alloc_section].m_next = m_memory_info->m_first_section;
		m_memory_info->m_first_section = alloc_section;
		CheckMemoryInfo();
		return alloc_section;
	}

	while (true)
	{
		TAssert(curr_section >= 0);

		if (m_sections[curr_section].m_next < 0)
		{
			// This is the last one, we can allocate stuff after it.

			int32 start = m_sections[curr_section].m_start + m_sections[curr_section].m_length;

			TAssert(start <= m_memory_size);

			// Return memory aligned to a cache line.
			int remainder = stb_mod_eucl(start, 64);
			if (remainder)
				start = start - remainder + 64;

			int32 bytes_remaining = (int32)m_memory_size - start;

			if (bytes_remaining < size)
			{
				if (!ResizeBackingMemory(size))
					return -1;

				bytes_remaining = (int32)m_memory_size - start;
			}

			TAssert(bytes_remaining >= size);
			m_sections[alloc_section].m_start = start;
			m_sections[alloc_section].m_length = size;
			m_sections[alloc_section].m_next = -1;
			m_sections[curr_section].m_next = alloc_section;
			CheckMemoryInfo();
			return alloc_section;
		}
		else
		{
			int next_section = m_sections[curr_section].m_next;
			int32 next_start = m_sections[next_section].m_start;
			int32 this_start = m_sections[curr_section].m_start;
			int32 this_length = m_sections[curr_section].m_length;

			if (next_start - (this_start + this_length) > size)
			{
				// If every allocation is 64 aligned and rounded
				// up to the next 64 bytes then this should be
				// aligned already for free.
				TAssert(stb_mod_eucl(this_start + this_length, 64) == 0);

				// We've found a gap! Stick it in here.
				m_sections[alloc_section].m_start = this_start + this_length;
				m_sections[alloc_section].m_length = size;
				m_sections[alloc_section].m_next = next_section;
				m_sections[curr_section].m_next = alloc_section;
				CheckMemoryInfo();
				return alloc_section;
			}

			curr_section = next_section;
		}
	}

	return 0;
}

void SectionAllocator::Free(int32 section)
{
	CheckMemoryInfo();

	if (m_memory_info->m_first_section == section)
	{
		m_sections[m_memory_info->m_first_section].m_length = 0;
		m_memory_info->m_first_section = m_sections[m_memory_info->m_first_section].m_next;
		return;
	}

	int8 last_section = m_memory_info->m_first_section;
	int8 curr_section = m_sections[m_memory_info->m_first_section].m_next;
	while (curr_section != section)
	{
		last_section = curr_section;
		curr_section = m_sections[last_section].m_next;

		TAssert(curr_section >= 0);
	}

	TAssert(curr_section == section); // If not, we tried to free an un-allocated section.

	m_sections[curr_section].m_length = 0;
	m_sections[last_section].m_next = m_sections[curr_section].m_next;

	CheckMemoryInfo();
}

bool SectionAllocator::ResizeBackingMemory(int32 allocation_size)
{
	if (!m_rcp)
		return false;

	int prev_memory_size = m_memory_size;
	m_memory_size = m_rcp(this, allocation_size);

	// If you hit this the reallocated buffer is too small.
	TAssert(m_memory_size >= prev_memory_size + allocation_size);
	return m_memory_size >= prev_memory_size + allocation_size;
}

void SectionAllocator::CheckMemoryInfo()
{
#ifdef _DEBUG
	int8 curr_section = m_memory_info->m_first_section;

	while (-1 != curr_section)
	{
		TAssert(m_sections[curr_section].m_length > 0);
		TAssert(stb_mod_eucl(m_sections[curr_section].m_start, 64) == 0);
		TAssert(stb_mod_eucl(m_sections[curr_section].m_length, 64) == 0);

		if (m_sections[curr_section].m_next >= 0)
			TAssert(m_sections[m_sections[curr_section].m_next].m_start >= m_sections[curr_section].m_start + m_sections[curr_section].m_length);

		curr_section = m_sections[curr_section].m_next;
	}
#endif
}
