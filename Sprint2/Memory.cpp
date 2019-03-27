#include "Memory.h"

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