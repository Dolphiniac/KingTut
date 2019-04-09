#pragma once

#include "Renderer.h"
#include "Memory.h"

// Simple mesh class to hold interleaved vertex format and short index data
class Mesh {
public:
	static Mesh * Create( const vertex_t * vertexData, uint32_t vertexSize, const uint16_t * indexData, uint32_t indexSize, uint32_t indexCount );
	VkBuffer GetVertexBuffer() const { return m_vertexBuffer; }
	VkBuffer GetIndexBuffer() const { return m_indexBuffer; }
	uint32_t GetIndexCount() const { return m_indexCount; }

private:
	VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
	allocation_t m_vertexBufferMemory = {};
	VkBuffer m_indexBuffer = VK_NULL_HANDLE;
	allocation_t m_indexBufferMemory = {};
	uint32_t m_indexCount = 0;

private:
	Mesh() = default;
};