#include "Buffer.h"
#include <string.h>

Buffer * Buffer::Create( void * data, uint32_t dataSize ) {
	Buffer * result = new Buffer;

	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
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