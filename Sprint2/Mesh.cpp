#include "Mesh.h"
#include <string.h>

Mesh * Mesh::Create( const vertex_t * vertexData, uint32_t vertexSize, const uint16_t * indexData, uint32_t indexSize, uint32_t indexCount ) {
	Mesh * result = new Mesh;

	// Create the buffers to be bound to the context
	VkBufferCreateInfo vertexBufferCreateInfo = {};
	vertexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertexBufferCreateInfo.size = vertexSize;
	vertexBufferCreateInfo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	VK_CHECK( vkCreateBuffer( renderObjects.device, &vertexBufferCreateInfo, NULL, &result->m_vertexBuffer ) );

	VkBufferCreateInfo indexBufferCreateInfo = {};
	indexBufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	indexBufferCreateInfo.size = indexSize;
	indexBufferCreateInfo.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	VK_CHECK( vkCreateBuffer( renderObjects.device, &indexBufferCreateInfo, NULL, &result->m_indexBuffer ) );

	// Allocate the backing memory for the buffers.  Like non-swapchain images, memory must be separately allocated from the
	// resource and bound to it.  This allows aliasing and user-controlled memory allocation schemes.
	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements( renderObjects.device, result->m_vertexBuffer, &memoryRequirements );
	AllocateDeviceMemory( memoryRequirements, MEMORY_MAPPABLE, result->m_vertexBufferMemory );
	VK_CHECK( vkBindBufferMemory( renderObjects.device, result->m_vertexBuffer, result->m_vertexBufferMemory.memory, result->m_vertexBufferMemory.offset ) );

	vkGetBufferMemoryRequirements( renderObjects.device, result->m_indexBuffer, &memoryRequirements );
	AllocateDeviceMemory( memoryRequirements, MEMORY_MAPPABLE, result->m_indexBufferMemory );
	VK_CHECK( vkBindBufferMemory( renderObjects.device, result->m_indexBuffer, result->m_indexBufferMemory.memory, result->m_indexBufferMemory.offset ) );

	// Map the memory and copy the data in
	void * memory;
	VK_CHECK( vkMapMemory( renderObjects.device, result->m_vertexBufferMemory.memory, result->m_vertexBufferMemory.offset, vertexSize, 0, &memory ) );
	memcpy( memory, vertexData, vertexSize );
	vkUnmapMemory( renderObjects.device, result->m_vertexBufferMemory.memory );

	VK_CHECK( vkMapMemory( renderObjects.device, result->m_indexBufferMemory.memory, result->m_indexBufferMemory.offset, indexSize, 0, &memory ) );
	memcpy( memory, indexData, indexSize );
	vkUnmapMemory( renderObjects.device, result->m_indexBufferMemory.memory );

	// Store the index count so it can be retrieved for an indexed draw call
	result->m_indexCount = indexCount;

	return result;
}