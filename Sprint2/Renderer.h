#pragma once

#include <vulkan.h>
#include <assert.h>

#if defined( ARRAY_COUNT )
#undef ARRAY_COUNT
#endif
#define ARRAY_COUNT( x ) ( sizeof( ( x ) ) / sizeof( ( x )[ 0 ] ) )

#if defined( BIT )
#undef BIT
#endif
#define BIT( x ) ( 1 << ( x ) )

#define VK_CHECK( call ) assert( ( call ) == VK_SUCCESS )

const uint32_t SWAPCHAIN_IMAGE_COUNT = 2;

class Image;
class CommandContext;

struct renderObjects_t {
	VkInstance							instance;
	VkPhysicalDevice					physicalDevice;
	VkPhysicalDeviceMemoryProperties	memoryProperties;
	VkDevice							device;
	uint32_t							queueFamilyIndex;
	VkQueue								queue;
	VkSurfaceKHR						surface;
	VkExtent2D							swapchainExtent;
	VkFormat							swapchainFormat;
	VkSwapchainKHR						swapchain;
	VkFramebuffer						framebuffers[ SWAPCHAIN_IMAGE_COUNT ];
	VkCommandPool						commandPool;
	CommandContext *					commandContext;
	VkFence								renderFence;
	VkSemaphore							imageAcquireSemaphore;
	VkSemaphore							renderCompleteSemaphore;
	VkPipelineLayout					emptyLayout;

	Image *								colorImage;
	Image *								depthImage;
	Image *								swapchainImage;
};

extern renderObjects_t renderObjects;

void Renderer_Init();