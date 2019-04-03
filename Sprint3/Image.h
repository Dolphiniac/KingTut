#pragma once

#include "Renderer.h"
#include "Memory.h"

enum imageFormat_t {
	IMAGE_FORMAT_RGBA8,
	IMAGE_FORMAT_DEPTH,
};

enum imageUsageFlags_t {
	IMAGE_USAGE_RENDER_TARGET	= BIT( 0 ),
	IMAGE_USAGE_SHADER			= BIT( 1 ),
};
inline imageUsageFlags_t operator |( imageUsageFlags_t left, imageUsageFlags_t right ) {
	return ( imageUsageFlags_t )( ( int )left | ( int )right );
}

class Swapchain;

class Image {
public:
	static Image * Create( uint32_t width, uint32_t height, imageFormat_t format, imageUsageFlags_t usage );
	static Image * CreateFromSwapchain();
	static Image * CreateFromFile( const char * filename );
	imageFormat_t GetFormat() const { return m_format; }
	uint32_t GetWidth() const { return m_width; }
	uint32_t GetHeight() const { return m_height; }
	VkImage GetImage() const { return m_image; }
	VkImageView GetView() const { return m_imageView; }
	void SelectSwapchainImage( uint32_t index );

private:
	imageFormat_t m_format;
	VkImage m_image = VK_NULL_HANDLE;
	allocation_t m_memory = {};
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