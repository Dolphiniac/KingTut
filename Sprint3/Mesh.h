#pragma once

#include "Renderer.h"
#include "Memory.h"

class Buffer;

class Mesh {
public:
	static Mesh * Create( const vertex_t * vertexData, uint32_t vertexSize, const uint16_t * indexData, uint32_t indexSize, uint32_t indexCount );
	const Buffer * GetVertexBuffer() const { return m_vertexBuffer; }
	const Buffer * GetIndexBuffer() const { return m_indexBuffer; }
	uint32_t GetIndexCount() const { return m_indexCount; }

private:
	Buffer * m_vertexBuffer = NULL;
	Buffer * m_indexBuffer = NULL;
	uint32_t m_indexCount = 0;

private:
	Mesh() = default;
};