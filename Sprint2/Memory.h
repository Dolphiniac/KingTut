#pragma once

#include "Renderer.h"

struct allocation_t {
	VkDeviceMemory	memory;
	uint64_t		offset;
};

enum memoryOptions_t {
	MEMORY_NONE = 0,
	MEMORY_MAPPABLE = BIT( 0 ),
};

void AllocateDeviceMemory( const VkMemoryRequirements & memoryRequirements, memoryOptions_t options, allocation_t & allocation );