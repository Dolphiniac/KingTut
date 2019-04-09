#pragma once

#include "Renderer.h"

// An allocation just needs the memory object and the offset into it.  For this project, the offset will always be 0, because the allocator is dead simple.
struct allocation_t {
	VkDeviceMemory	memory;
	uint64_t		offset;
};

enum memoryOptions_t {
	MEMORY_NONE = 0,
	MEMORY_MAPPABLE = BIT( 0 ),
};

// Given the memory requirements and options, return an allocation large enough to hold the resource.
void AllocateDeviceMemory( const VkMemoryRequirements & memoryRequirements, memoryOptions_t options, allocation_t & allocation );