#pragma once

#include "Renderer.h"
#include "Memory.h"

enum bufferUsageFlags_t {
	BUFFER_USAGE_UNIFORM_BUFFER = BIT( 0 ),
	BUFFER_USAGE_VERTEX_BUFFER = BIT( 1 ),
	BUFFER_USAGE_INDEX_BUFFER = BIT( 2 ),
};
inline bufferUsageFlags_t operator |( bufferUsageFlags_t left, bufferUsageFlags_t right ) {
	return ( bufferUsageFlags_t )( ( int )left | ( int )right );
}

class Buffer {
public:
	static Buffer * Create( const void * data, uint32_t dataSize, bufferUsageFlags_t usage );
	VkBuffer GetBuffer() const { return m_buffer; }

private:
	VkBuffer m_buffer = VK_NULL_HANDLE;
	allocation_t m_memory = {};

private:
	Buffer() = default;
};