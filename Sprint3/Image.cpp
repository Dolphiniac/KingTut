#include "Image.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.c"

VkFormat TranslateFormat( imageFormat_t format ) {
	switch ( format ) {
		case IMAGE_FORMAT_RGBA8: {
			return VK_FORMAT_R8G8B8A8_UNORM;
		}
		case IMAGE_FORMAT_DEPTH: {
			return VK_FORMAT_D32_SFLOAT;
		}
	}
	return VK_FORMAT_UNDEFINED;
}

static VkImageUsageFlags TranslateUsage( imageUsageFlags_t usage, imageFormat_t format ) {
	VkImageUsageFlags result = 0;
	if ( ( usage & IMAGE_USAGE_RENDER_TARGET ) != 0 ) {
		if ( format == IMAGE_FORMAT_DEPTH ) {
			result |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		} else {
			result |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		}
	}
	if ( ( usage & IMAGE_USAGE_SHADER ) != 0 ) {
		result |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		result |= VK_IMAGE_USAGE_SAMPLED_BIT;
	}
	return result;
}

static VkImageAspectFlags TranslateFormatToAspect( imageFormat_t format ) {
	VkImageAspectFlags result = 0;
	if ( format == IMAGE_FORMAT_DEPTH ) {
		result |= VK_IMAGE_ASPECT_DEPTH_BIT;
	} else {
		result |= VK_IMAGE_ASPECT_COLOR_BIT;
	}
	return result;
}

Image * Image::Create( uint32_t width, uint32_t height, imageFormat_t format, imageUsageFlags_t usage ) {
	Image * result = new Image;
	result->m_format = format;
	result->m_width = width;
	result->m_height = height;
	VkImageCreateInfo imageCreateInfo = {};
	imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.arrayLayers = 1;
	imageCreateInfo.extent.depth = 1;
	imageCreateInfo.extent.width = width;
	imageCreateInfo.extent.height = height;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.format = TranslateFormat( format );
	imageCreateInfo.usage = TranslateUsage( usage, format );
	VK_CHECK( vkCreateImage( renderObjects.device, &imageCreateInfo, NULL, &result->m_image ) );

	VkMemoryRequirements memReq;
	vkGetImageMemoryRequirements( renderObjects.device, result->m_image, &memReq );

	AllocateDeviceMemory( memReq, MEMORY_NONE, result->m_memory );
	VK_CHECK( vkBindImageMemory( renderObjects.device, result->m_image, result->m_memory.memory, result->m_memory.offset ) );

	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
	};
	viewCreateInfo.format = imageCreateInfo.format;
	viewCreateInfo.image = result->m_image;
	viewCreateInfo.subresourceRange.aspectMask = TranslateFormatToAspect( format );
	viewCreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	viewCreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	VK_CHECK( vkCreateImageView( renderObjects.device, &viewCreateInfo, NULL, &result->m_imageView ) );

	return result;
}

Image * Image::CreateFromSwapchain() {
	Image * result = new Image;
	result->m_width = renderObjects.swapchainExtent.width;
	result->m_height = renderObjects.swapchainExtent.height;

	uint32_t swapchainImageCount;
	VK_CHECK( vkGetSwapchainImagesKHR( renderObjects.device, renderObjects.swapchain, &swapchainImageCount, NULL ) );
	assert( swapchainImageCount == SWAPCHAIN_IMAGE_COUNT );
	VK_CHECK( vkGetSwapchainImagesKHR( renderObjects.device, renderObjects.swapchain, &swapchainImageCount, result->m_swapchainImages ) );

	VkImageViewCreateInfo viewCreateInfo = {};
	viewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewCreateInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
	};
	viewCreateInfo.format = renderObjects.swapchainFormat;
	viewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewCreateInfo.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;
	viewCreateInfo.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	for ( uint32_t i = 0; i < swapchainImageCount; ++i ) {
		viewCreateInfo.image = result->m_swapchainImages[ i ];
		VK_CHECK( vkCreateImageView( renderObjects.device, &viewCreateInfo, NULL, &result->m_swapchainViews[ i ] ) );
	}

	return result;
}

Image * Image::CreateFromFile( const char * filename ) {
	int x;
	int y;
	int comp;
	uint8_t * imageData = stbi_load( filename, &x, &y, &comp, 4 );
	Image * result = Create( x, y, IMAGE_FORMAT_RGBA8, IMAGE_USAGE_SHADER );
	VkMemoryRequirements requirements;
	vkGetImageMemoryRequirements( renderObjects.device, result->GetImage(), &requirements );
	StageImageData( imageData, x * y * 4, ( uint32_t )requirements.alignment, result );
	stbi_image_free( imageData );

	return result;
}

void Image::SelectSwapchainImage( uint32_t index ) {
	m_image = m_swapchainImages[ index ];
	m_imageView = m_swapchainViews[ index ];
}