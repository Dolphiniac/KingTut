#pragma once

#include "Renderer.h"

const uint32_t COMMAND_BUFFER_COUNT = 2;
const uint32_t RENDER_TARGET_COUNT = 2;

class Image;

class CommandContext {
public:
	static CommandContext * Create();
	void SetRenderTargets( uint32_t colorTargetCount, Image * colorTargets[ RENDER_TARGET_COUNT ], Image * depthStencilTarget );

private:
	VkCommandPool m_commandPool = VK_NULL_HANDLE;
	VkCommandBuffer m_commandBuffer = VK_NULL_HANDLE;
	VkFence m_fence = VK_NULL_HANDLE;

	VkCommandBuffer m_commandBuffers[ COMMAND_BUFFER_COUNT ] = { VK_NULL_HANDLE };
	VkFence m_fences[ COMMAND_BUFFER_COUNT ] = { VK_NULL_HANDLE };
	uint32_t m_currentBufferIndex = 0;
	bool m_inRenderPass = false;

private:
	CommandContext() = default;
};