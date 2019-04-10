#pragma once

#include <vulkan.h>
#include <assert.h>
#include "Common.h"

#define VK_CHECK( call ) assert( ( call ) == VK_SUCCESS )
#define VK_FOREVER ( ~0U )

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

struct Matrix44 {
	float x[ 16 ];
};

const uint32_t SWAPCHAIN_IMAGE_COUNT = 2;

class Image;
class CommandContext;

enum samplerType_t {
	SAMPLER_TYPE_LINEAR,
	SAMPLER_TYPE_COUNT
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
	uint32_t							swapchainImageIndex;
	VkDescriptorPool					descriptorPool;
	VkDescriptorSetLayout				frameDescriptorSetLayout;
	VkDescriptorSetLayout				viewDescriptorSetLayout;
	VkDescriptorSetLayout				meshDescriptorSetLayout;
	VkPipelineLayout					unifiedPipelineLayout;
	VkSampler							samplers[ SAMPLER_TYPE_COUNT ];
	
	Image *								colorImage;
	Image *								depthImage;
	Image *								swapchainImage;
};

extern renderObjects_t renderObjects;

void Renderer_Init();
void Renderer_BeginFrame();
void Renderer_AcquireSwapchainImage();
void Renderer_EndFrame();