#pragma once

#include <vulkan.h>
#include <assert.h>
#include "Common.h"

#define VK_CHECK( call ) assert( ( call ) == VK_SUCCESS )
#define VK_FOREVER ( ~0U )

const uint32_t SWAPCHAIN_IMAGE_COUNT = 2;

class Image;
class CommandContext;

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
	uint32_t							swapchainImageIndex;

	Image *								colorImage;
	Image *								depthImage;
	Image *								swapchainImage;
};

extern renderObjects_t renderObjects;

// Call before creation or usage of any renderer objects
void Renderer_Init();
// Call before any render commands in a frame (hint: CommandContext non-static methods)
void Renderer_BeginFrame();
// Call between BeginFrame and EndFrame, but before using the swapchainImage
void Renderer_AcquireSwapchainImage();
// Call at the end of the frame to submit commands and present
void Renderer_EndFrame();