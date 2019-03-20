#pragma once

#include "Renderer.h"

#define BIT( x ) ( 1 << ( x ) )

enum imageFormat_t {
	IMAGE_FORMAT_RGBA8,
	IMAGE_FORMAT_DEPTH,
};

enum imageUsageFlags_t {
	IMAGE_USAGE_RENDER_TARGET	= BIT( 0 ),
};

class Swapchain;

class Image {
public:
	static Image * Create( uint32_t width, uint32_t height, imageFormat_t format, imageUsageFlags_t usage );
	static Image * CreateFromSwapchain();
	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	VkImageView GetView() const { return m_imageView; }

private:
	VkImage m_image = VK_NULL_HANDLE;
	VkDeviceMemory m_memory = VK_NULL_HANDLE;
	VkImageView m_imageView = VK_NULL_HANDLE;

	uint32_t m_width = 0;
	uint32_t m_height = 0;

	// These are specific to the Image created from the swapchain.
	// This causes 32B of bloat for non-swapchain images, which will be most, but it's not that crucial
	VkImage m_swapchainImages[ SWAPCHAIN_IMAGE_COUNT ] = { VK_NULL_HANDLE };
	VkImageView m_swapchainViews[ SWAPCHAIN_IMAGE_COUNT ] = { VK_NULL_HANDLE };

private:
	Image() = default;
};