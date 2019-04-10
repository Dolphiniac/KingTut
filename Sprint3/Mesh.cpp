#include "Mesh.h"
#include "Buffer.h"
#include <string.h>

Mesh * Mesh::Create( const vertex_t * vertexData, uint32_t vertexSize, const uint16_t * indexData, uint32_t indexSize, uint32_t indexCount ) {
	Mesh * result = new Mesh;

	// Now that we have a Buffer class, the Mesh class is dead simple.
	result->m_vertexBuffer = Buffer::Create( vertexData, vertexSize, BUFFER_USAGE_VERTEX_BUFFER );
	result->m_indexBuffer = Buffer::Create( indexData, indexSize, BUFFER_USAGE_INDEX_BUFFER );

	result->m_indexCount = indexCount;

	return result;
}