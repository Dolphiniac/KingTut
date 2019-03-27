#include "Mesh.h"
#include <string.h>

Mesh * Mesh::Create( const void * vertexData, uint32_t vertexSize, const void * indexData, uint32_t indexSize ) {
	Mesh * result = new Mesh;

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

	VkMemoryRequirements memoryRequirements;
	vkGetBufferMemoryRequirements( renderObjects.device, result->m_vertexBuffer, &memoryRequirements );
	AllocateDeviceMemory( memoryRequirements, MEMORY_MAPPABLE, result->m_vertexBufferMemory );
	VK_CHECK( vkBindBufferMemory( renderObjects.device, result->m_vertexBuffer, result->m_vertexBufferMemory.memory, result->m_vertexBufferMemory.offset ) );

	vkGetBufferMemoryRequirements( renderObjects.device, result->m_indexBuffer, &memoryRequirements );
	AllocateDeviceMemory( memoryRequirements, MEMORY_MAPPABLE, result->m_indexBufferMemory );
	VK_CHECK( vkBindBufferMemory( renderObjects.device, result->m_indexBuffer, result->m_indexBufferMemory.memory, result->m_indexBufferMemory.offset ) );

	void * memory;
	VK_CHECK( vkMapMemory( renderObjects.device, result->m_vertexBufferMemory.memory, result->m_vertexBufferMemory.offset, vertexSize, 0, &memory ) );
	memcpy( memory, vertexData, vertexSize );
	vkUnmapMemory( renderObjects.device, result->m_vertexBufferMemory.memory );

	VK_CHECK( vkMapMemory( renderObjects.device, result->m_indexBufferMemory.memory, result->m_indexBufferMemory.offset, indexSize, 0, &memory ) );
	memcpy( memory, indexData, indexSize );
	vkUnmapMemory( renderObjects.device, result->m_indexBufferMemory.memory );

	return result;
}