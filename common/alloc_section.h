#pragma once

#include "common.h"

// Section allocator. Sections out a block of memory as requests come in.
// Expects a small (bounded by a constant) number of large allocations.
// Doesn't touch the actual memory, and so it is very suitable for managing
// memory blocks that will be moved around or written to disk.

struct MemorySection
{
	int32 m_start;
	int32 m_length;
	int8  m_next;
};

struct MemorySectionInfo
{
	int m_first_section;
};

// First argument is the section allocator requesting the resize.
// Second argument is the size of the allocation that triggered the resize.
// The new buffer should have size at least its current size plus allocation_size.
// It expects you to copy the data from old to new buffer in the same
// layout as what was there previously.
// Return value should be the size of the new buffer.
typedef int32 (*ResizeCallbackProcedure)(struct SectionAllocator*, int32 allocation_size);

struct SectionAllocator
{
	MemorySection* m_sections;
	MemorySectionInfo* m_memory_info;
	MemorySectionInfo  m_memory_info_storage;
	int   m_num_sections;

	int32 m_memory_size;

	ResizeCallbackProcedure m_rcp;

	SectionAllocator()
	{
		m_rcp = nullptr;
	}

	// You allocate sections and pass it in. All datakeeping goes on in sections.
	// SectionAllocator never touches the actual memory. In fact you don't even
	// have to pass it in. memory_size is the size of the memory to be sectioned out.
	void Initialize(MemorySection* sections, int num_sections, int32 memory_size, MemorySectionInfo* memory_info = nullptr);

	// Like Initialize but will not write initial values to the sections and memory info.
	// Suitable for if you are loading an existing SectionAllocator from eg disk.
	void LoadFrom(MemorySection* sections, int num_sections, int32 memory_size, MemorySectionInfo* memory_info);

	// If the allocator runs out of space, this callback will be called.
	void SetResizeCallback(ResizeCallbackProcedure rcp)
	{
		m_rcp = rcp;
	}

	// Request a section of memory.
	// If it returns negative, that means there was no space.
	int32 Alloc(int32 size);
	void Free(int32 section);

	bool ResizeBackingMemory(int32 allocation_size);

	void CheckMemoryInfo();
};
