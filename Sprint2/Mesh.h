#pragma once

#include "Renderer.h"
#include "Memory.h"

struct Vector2 {
	float x;
	float y;
};

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Vector4 {
	float x;
	float y;
	float z;
	float w;
};

struct vertex_t {
	Vector3 position;
	Vector2 uv;
	Vector4 color;
};

class Mesh {
public:
	static Mesh * Create( const void * vertexData, uint32_t vertexSize, const void * indexData, uint32_t indexSize, uint32_t indexCount );
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