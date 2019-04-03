#pragma once

#include "Renderer.h"
#include "Memory.h"

class Buffer {
public:
	static Buffer * Create( void * data, uint32_t dataSize );
	VkBuffer GetBuffer() const { return m_buffer; }

private:
	VkBuffer m_buffer = VK_NULL_HANDLE;
	allocation_t m_memory = {};

private:
	Buffer() = default;
};