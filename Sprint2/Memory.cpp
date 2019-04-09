#include "Memory.h"

void AllocateDeviceMemory( const VkMemoryRequirements & memoryRequirements, memoryOptions_t options, allocation_t & allocation ) {
	VkMemoryAllocateInfo memoryAllocateInfo = {};
	memoryAllocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	// Allocations take a memory type index that refer to a specific heap (implementation defined) with certain properties.
	// We use the properties and a bitmask of valid types for the resource to choose a type.
	memoryAllocateInfo.memoryTypeIndex = -1;

	VkMemoryPropertyFlags flags = 0;
	if ( ( options & MEMORY_MAPPABLE ) != 0 ) {
		flags |= VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;	// The memory is accessible (mappable) by the CPU
		flags |= VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;	// The memory does not require explicit synchronization between writes and reads
	} else {
		flags |= VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;	// The memory lives on the GPU.  Types with only this flag provide the fastest access by the GPU.
	}

	// This loop is straight out of the spec.  It picks the first valid memory type for the resource that has all of the flags we want.
	for ( uint32_t i = 0; i < renderObjects.memoryProperties.memoryTypeCount; ++i ) {
		if ( ( ( 1 << i ) & memoryRequirements.memoryTypeBits ) != 0 ) {
			if ( ( renderObjects.memoryProperties.memoryTypes[ i ].propertyFlags & flags ) == flags ) {
				memoryAllocateInfo.memoryTypeIndex = i;
				break;
			}
		}
	}

	VK_CHECK( vkAllocateMemory( renderObjects.device, &memoryAllocateInfo, NULL, &allocation.memory ) );
	allocation.offset = 0;	// For freshly allocated memory, offset will always be 0.  There are no memory blocks returned by Vulkan that don't meet alignment requirements for a resource.
}