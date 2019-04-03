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

struct stagingBuffer_t {
	VkBuffer buffer = VK_NULL_HANDLE;
	allocation_t memory = {};
	void * memoryData = NULL;
	uint32_t currentOffset = 0;
	VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
	bool inFrame = false;
};

extern stagingBuffer_t stagingBuffer;
class Buffer;
class Image;

void AllocateDeviceMemory( const VkMemoryRequirements & memoryRequirements, memoryOptions_t options, allocation_t & allocation );
void StageImageData( void * data, uint32_t size, uint32_t alignment, const Image * targetImage );
void BeginStagingFrame();
void EndStagingFrame();