#include "Memory.h"
#include "Image.h"
#include <string.h>

stagingBuffer_t stagingBuffer;

void AllocateDeviceMemory( const VkMemoryRequirements & memoryRequirements, memoryOptions_t options, allocation_t & allocation ) {
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	memoryAllocateInfo.memoryTypeIndex = -1;

	VkMemoryPropertyFlags flags = 0;
	if ( ( options & MEMORY_MAPPABLE ) != 0 ) {
		flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
		flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
	} else {
		flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	}

	for ( uint32_t i = 0; i < renderObjects.memoryProperties.memoryTypeCount; ++i ) {
		if ( ( ( 1 << i ) & memoryRequirements.memoryTypeBits ) != 0 ) {
			if ( ( renderObjects.memoryProperties.memoryTypes[ i ].propertyFlags & flags ) == flags ) {
				memoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
	}

	VK_CHECK( vkAllocateMemory( renderObjects.device, &memoryAllocateInfo, NULL, &allocation.memory ) );
	allocation.offset = 0;
}

void StageImageData( void * data, uint32_t size, uint32_t alignment, const Image * targetImage ) {
	if ( ( stagingBuffer.currentOffset % alignment ) != 0 ) {
		stagingBuffer.currentOffset += alignment - stagingBuffer.currentOffset % alignment;
	}
	memcpy( ( uint8_t * )stagingBuffer.memoryData + stagingBuffer.currentOffset, data, size );

	VkImageMemoryBarrier barrier = {};
	barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.srcAccessMask = 0;
	barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.image = targetImage->GetImage();
	barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	vkCmdPipelineBarrier( stagingBuffer.commandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier );
	
	VkBufferImageCopy region = {};
	region.bufferOffset = stagingBuffer.currentOffset;
	region.imageExtent.width = targetImage->GetWidth();
	region.imageExtent.height = targetImage->GetHeight();
	region.imageExtent.depth = 1;
	region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.layerCount = 1;
	vkCmdCopyBufferToImage( stagingBuffer.commandBuffer, stagingBuffer.buffer, targetImage->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region );

	barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	vkCmdPipelineBarrier( stagingBuffer.commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier );
}

void BeginStagingFrame() {
	if ( stagingBuffer.inFrame == true ) {
		return;
	}

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
	VK_CHECK( vkBeginCommandBuffer( stagingBuffer.commandBuffer, &beginInfo ) );
	stagingBuffer.inFrame = true;
}

void EndStagingFrame() {
	VK_CHECK( vkEndCommandBuffer( stagingBuffer.commandBuffer ) );
	stagingBuffer.inFrame = false;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &stagingBuffer.commandBuffer;
	VK_CHECK( vkQueueSubmit( renderObjects.queue, 1, &submitInfo, VK_NULL_HANDLE ) );
}