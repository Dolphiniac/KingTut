#include "Buffer.h"
#include <string.h>

static VkBufferUsageFlags TranslateUsage( bufferUsageFlags_t usage ) {
	VkBufferUsageFlags result = 0;
	if ( ( usage & BUFFER_USAGE_UNIFORM_BUFFER ) != 0 ) {
		result |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	}
	if ( ( usage & BUFFER_USAGE_VERTEX_BUFFER ) != 0 ) {
		result |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	}
	if ( ( usage & BUFFER_USAGE_INDEX_BUFFER ) != 0 ) {
		result |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	}

	return result;
}

Buffer * Buffer::Create( const void * data, uint32_t dataSize, bufferUsageFlags_t usage ) {
	Buffer * result = new Buffer;

	// This is basically the same thing that we did in Sprint 2 for the vertex and index buffers in Mesh::Create,
	// just generalized for different usages.
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = TranslateUsage( usage );
	bufferCreateInfo.size = dataSize;
	VK_CHECK( vkCreateBuffer( renderObjects.device, &bufferCreateInfo, NULL, &result->m_buffer ) );

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements( renderObjects.device, result->m_buffer, &memReq );
	AllocateDeviceMemory( memReq, MEMORY_MAPPABLE, result->m_memory );
	VK_CHECK( vkBindBufferMemory( renderObjects.device, result->m_buffer, result->m_memory.memory, result->m_memory.offset ) );

	if ( data != NULL ) {
		void * mem;
		VK_CHECK( vkMapMemory( renderObjects.device, result->m_memory.memory, result->m_memory.offset, dataSize, 0, &mem ) );
		memcpy( mem, data, dataSize );
		vkUnmapMemory( renderObjects.device, result->m_memory.memory );
	}

	return result;
}