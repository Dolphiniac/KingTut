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

enum imageUsageFlags_t {
	IMAGE_USAGE_RENDER_TARGET = BIT( 0 ),
	IMAGE_USAGE_TRANSFER_SRC = BIT( 1 ),
	IMAGE_USAGE_TRANSFER_DST = BIT( 2 ),
	IMAGE_USAGE_SHADER = BIT( 3 ),
};
inline imageUsageFlags_t operator |( imageUsageFlags_t left, imageUsageFlags_t right ) {
	return ( imageUsageFlags_t )( ( int )left | ( int )right );
}

extern stagingBuffer_t stagingBuffer;
class Buffer;
class Image;

void AllocateDeviceMemory( const VkMemoryRequirements & memoryRequirements, memoryOptions_t options, allocation_t & allocation );
// Copy the linear image data into the staging buffer and produce a copy command to fill the targetImage.
void StageImageData( void * data, uint32_t size, uint32_t alignment, const Image * targetImage );
// Start the command buffer and linear allocator in the staging buffer memory.
void BeginStagingFrame();
// Transition image to a proper non-undefined layout before first use.
void InitializeImageLayout( Image * image, imageUsageFlags_t usage );
// Do a special transition for all swapchain images.
void InitializeSwapchainImageLayout( Image * image );
// Finish the staging buffer for this frame and submit any commands.
void EndStagingFrame();