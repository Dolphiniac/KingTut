#include "Renderer.h"
#include "Image.h"
#include "CommandContext.h"
#include "DescriptorSet.h"
#include "Memory.h"
#include <vector>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

renderObjects_t renderObjects;

#pragma comment( lib, "vulkan-1" )

VkBool32 VKAPI_PTR DebugReportCallback( VkDebugReportFlagsEXT flags, 
										VkDebugReportObjectTypeEXT objectType, 
										uint64_t object, 
										size_t location, 
										int32_t messageCode, 
										const char * pLayerPrefix, 
										const char * pMessage, 
										void * pUserData ) {
	OutputDebugStringA( "VK: " );
	OutputDebugStringA( pMessage );
	OutputDebugStringA( "\n" );
	return VK_TRUE;
}

static void CreateInstance() {
	std::vector< const char * > instanceExtensionNames = {
		VK_KHR_SURFACE_EXTENSION_NAME,
#if defined( _DEBUG )
		VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
#endif
	};
	extern const char * GetPlatformSurfaceExtensionName();
	instanceExtensionNames.push_back( GetPlatformSurfaceExtensionName() );
	std::vector< const char * > instanceLayerNames = {
#if defined( _DEBUG )
		"VK_LAYER_LUNARG_standard_validation",
#endif
	};

	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan test app";
	appInfo.pEngineName = "Mark II renderer";
	appInfo.applicationVersion = VK_MAKE_VERSION( 0, 2, 0 );
	appInfo.engineVersion = VK_MAKE_VERSION( 0, 2, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_0;

	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.enabledLayerCount = ( uint32_t )instanceLayerNames.size();;
	instanceCreateInfo.ppEnabledLayerNames = instanceLayerNames.data();
	instanceCreateInfo.enabledExtensionCount = ( uint32_t )instanceExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNames.data();
	VK_CHECK( vkCreateInstance( &instanceCreateInfo, NULL, &renderObjects.instance ) );
#if defined( _DEBUG )
	VkDebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo = {};
	debugReportCallbackCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	debugReportCallbackCreateInfo.flags = VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_DEBUG_BIT_EXT | VK_DEBUG_REPORT_INFORMATION_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT;
	debugReportCallbackCreateInfo.pfnCallback = DebugReportCallback;
	VkDebugReportCallbackEXT callback;
	PFN_vkCreateDebugReportCallbackEXT CreateDebugReportCallbackEXT = ( PFN_vkCreateDebugReportCallbackEXT )vkGetInstanceProcAddr( renderObjects.instance, "vkCreateDebugReportCallbackEXT" );
	VK_CHECK( CreateDebugReportCallbackEXT( renderObjects.instance, &debugReportCallbackCreateInfo, NULL, &callback ) );
#endif
}

static void GetPhysicalDevice() {
	uint32_t physicalDeviceCount;
	VK_CHECK( vkEnumeratePhysicalDevices( renderObjects.instance, &physicalDeviceCount, NULL ) );
	VkPhysicalDevice * allPhysicalDevices = new VkPhysicalDevice[ physicalDeviceCount ];
	VK_CHECK( vkEnumeratePhysicalDevices( renderObjects.instance, &physicalDeviceCount, allPhysicalDevices ) );
	uint32_t hardwareVendors[] = {
		0x1022,
		0x1002, // AMD
		0x10DE, // NVIDIA
		8086, // Intel
	};
	for ( uint32_t i = 0; i < physicalDeviceCount; ++i ) {
		VkPhysicalDeviceProperties props;
		vkGetPhysicalDeviceProperties( allPhysicalDevices[ i ], &props );
		bool foundHardwareVendor = false;
		for ( uint32_t j = 0; j < ARRAY_COUNT( hardwareVendors ); ++j ) {
			if ( props.vendorID == hardwareVendors[ j ] ) {
				foundHardwareVendor = true;
				break;
			}
		}
		if ( foundHardwareVendor == true ) {
			renderObjects.physicalDevice = allPhysicalDevices[ i ];
			break;
		}
	}
	// We're not going to worry about not getting a physical device here.  It'll break if it's still NULL (the default value) when we use it
	delete[] allPhysicalDevices; // They're just handles.  This doesn't actually invalidate any of the physical devices
	vkGetPhysicalDeviceMemoryProperties( renderObjects.physicalDevice, &renderObjects.memoryProperties );
}

static void CreateDevice() {
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties( renderObjects.physicalDevice, &queueFamilyCount, NULL );
	VkQueueFamilyProperties * queueFamilyProperties = new VkQueueFamilyProperties[ queueFamilyCount ];
	vkGetPhysicalDeviceQueueFamilyProperties( renderObjects.physicalDevice, &queueFamilyCount, queueFamilyProperties );
	renderObjects.queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	VkQueueFlags desiredQueueCaps = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	for ( uint32_t i = 0; i < queueFamilyCount; ++i ) {
		if ( ( queueFamilyProperties[ i ].queueFlags & desiredQueueCaps ) == desiredQueueCaps ) {
			renderObjects.queueFamilyIndex = i;
			break;
		}
	}
	delete[] queueFamilyProperties;
	float queuePriority = 1.0f;	// This value must be between 0 and 1, by spec, but it's unimportant, because we'll only create the one
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = renderObjects.queueFamilyIndex;
	queueCreateInfo.queueCount = 1;
	queueCreateInfo.pQueuePriorities = &queuePriority;
	const char * deviceExtensionNames[] = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	};
	VkDeviceCreateInfo deviceCreateInfo = {};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreateInfo.queueCreateInfoCount = 1;
	deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
	deviceCreateInfo.enabledExtensionCount = ARRAY_COUNT( deviceExtensionNames );
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames;
	VK_CHECK( vkCreateDevice( renderObjects.physicalDevice, &deviceCreateInfo, NULL, &renderObjects.device ) );
	vkGetDeviceQueue( renderObjects.device, renderObjects.queueFamilyIndex, 0, &renderObjects.queue );
}

static void CreateSwapchain() {
	VkBool32 presentSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR( renderObjects.physicalDevice, renderObjects.queueFamilyIndex, renderObjects.surface, &presentSupported );
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	VK_CHECK( vkGetPhysicalDeviceSurfaceCapabilitiesKHR( renderObjects.physicalDevice, renderObjects.surface, &surfaceCapabilities ) );
	renderObjects.swapchainExtent = surfaceCapabilities.currentExtent;
	uint32_t surfaceFormatCount;
	VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( renderObjects.physicalDevice, renderObjects.surface, &surfaceFormatCount, NULL ) );
	VkSurfaceFormatKHR * surfaceFormats = new VkSurfaceFormatKHR[ surfaceFormatCount ];
	VK_CHECK( vkGetPhysicalDeviceSurfaceFormatsKHR( renderObjects.physicalDevice, renderObjects.surface, &surfaceFormatCount, surfaceFormats ) );
	uint32_t presentModeCount;
	VK_CHECK( vkGetPhysicalDeviceSurfacePresentModesKHR( renderObjects.physicalDevice, renderObjects.surface, &presentModeCount, NULL ) );
	VkPresentModeKHR * presentModes = new VkPresentModeKHR[ presentModeCount ];
	VK_CHECK( vkGetPhysicalDeviceSurfacePresentModesKHR( renderObjects.physicalDevice, renderObjects.surface, &presentModeCount, presentModes ) );
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;	// This is required to be supported, so we'll use it as a fallback
	for ( uint32_t i = 0; i < presentModeCount; ++i ) {
		if ( presentModes[ i ] == VK_PRESENT_MODE_FIFO_RELAXED_KHR ) {
			presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			break;
		}
	}
	renderObjects.swapchainFormat = surfaceFormats[ 0 ].format;
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = renderObjects.surface;
	swapchainCreateInfo.minImageCount = SWAPCHAIN_IMAGE_COUNT;	// Technically, we should check this against the surface capabilities, but we're probably fine with a sane number like 2 or 3
	swapchainCreateInfo.imageFormat = surfaceFormats[ 0 ].format;
	swapchainCreateInfo.imageColorSpace = surfaceFormats[ 0 ].colorSpace;	// These are truly unimportant.  What you'll normally get is an SRGB nonlinear color space, and a BGRA8 format.  These are fine
	swapchainCreateInfo.imageExtent = renderObjects.swapchainExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;	// We can expect the surface to support this capability
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// Same as above
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_FALSE;
	VK_CHECK( vkCreateSwapchainKHR( renderObjects.device, &swapchainCreateInfo, NULL, &renderObjects.swapchain ) );
	delete[] presentModes;
	delete[] surfaceFormats;
}

static void CreateCommandBuffers() {
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = renderObjects.queueFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK( vkCreateCommandPool( renderObjects.device, &commandPoolCreateInfo, NULL, &renderObjects.commandPool ) );

	renderObjects.commandContext = CommandContext::Create();
}

static void CreateSynchronizationPrimitives() {
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	VK_CHECK( vkCreateFence( renderObjects.device, &fenceCreateInfo, NULL, &renderObjects.renderFence ) );

	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VK_CHECK( vkCreateSemaphore( renderObjects.device, &semaphoreCreateInfo, NULL, &renderObjects.imageAcquireSemaphore ) );
	VK_CHECK( vkCreateSemaphore( renderObjects.device, &semaphoreCreateInfo, NULL, &renderObjects.renderCompleteSemaphore ) );
}

static void CreateRenderTargets() {
	renderObjects.colorImage = Image::Create( 1920, 1080, IMAGE_FORMAT_RGBA8, IMAGE_USAGE_RENDER_TARGET | IMAGE_USAGE_SHADER );
	renderObjects.depthImage = Image::Create( 1920, 1080, IMAGE_FORMAT_DEPTH, IMAGE_USAGE_RENDER_TARGET );
	renderObjects.swapchainImage = Image::CreateFromSwapchain();
}

static void CreateUnifiedPipelineLayout() {
	VkDescriptorSetLayoutCreateInfo setLayoutCreateInfo = {};
	setLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	std::vector< VkDescriptorSetLayoutBinding > bindings;
	uint32_t currentBinding = 0;
	VkDescriptorSetLayoutBinding binding = {};
	binding.descriptorCount = 1;
	binding.stageFlags = VK_SHADER_STAGE_ALL;
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	for ( ; currentBinding < FRAME_DESCRIPTOR_UNIFORM_BUFFER_SLOT_BOUND; ++currentBinding ) {
		binding.binding = currentBinding;
		bindings.push_back( binding );
	}
	setLayoutCreateInfo.bindingCount = ( uint32_t )bindings.size();
	setLayoutCreateInfo.pBindings = bindings.data();
	VK_CHECK( vkCreateDescriptorSetLayout( renderObjects.device, &setLayoutCreateInfo, NULL, &renderObjects.frameDescriptorSetLayout ) );

	currentBinding = 0;
	bindings.clear();
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	for ( ; currentBinding < VIEW_DESCRIPTOR_UNIFORM_BUFFER_SLOT_BOUND; ++currentBinding ) {
		binding.binding = currentBinding;
		bindings.push_back( binding );
	}
	setLayoutCreateInfo.bindingCount = ( uint32_t )bindings.size();
	setLayoutCreateInfo.pBindings = bindings.data();
	VK_CHECK( vkCreateDescriptorSetLayout( renderObjects.device, &setLayoutCreateInfo, NULL, &renderObjects.viewDescriptorSetLayout ) );

	currentBinding = 0;
	bindings.clear();
	binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	for ( ; currentBinding < MESH_DESCRIPTOR_UNIFORM_BUFFER_SLOT_BOUND; ++currentBinding ) {
		binding.binding = currentBinding;
		bindings.push_back( binding );
	}
	binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	for ( ; currentBinding < MESH_DESCRIPTOR_SAMPLER_SLOT_BOUND; ++currentBinding ) {
		binding.binding = currentBinding;
		bindings.push_back( binding );
	}
	setLayoutCreateInfo.bindingCount = ( uint32_t )bindings.size();
	setLayoutCreateInfo.pBindings = bindings.data();
	VK_CHECK( vkCreateDescriptorSetLayout( renderObjects.device, &setLayoutCreateInfo, NULL, &renderObjects.meshDescriptorSetLayout ) );

	VkDescriptorSetLayout layouts[] = {
		renderObjects.frameDescriptorSetLayout,
		renderObjects.viewDescriptorSetLayout,
		renderObjects.meshDescriptorSetLayout,
	};
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutCreateInfo.setLayoutCount = ARRAY_COUNT( layouts );
	pipelineLayoutCreateInfo.pSetLayouts = layouts;
	VK_CHECK( vkCreatePipelineLayout( renderObjects.device, &pipelineLayoutCreateInfo, NULL, &renderObjects.unifiedPipelineLayout ) );
}

static void CreateDescriptorPool() {
	const uint32_t unifiedCount = 64 * 1024;

	VkDescriptorPoolSize poolSizes[] = {
		{
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			unifiedCount,
		},
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			unifiedCount,
		},
	};

	VkDescriptorPoolCreateInfo poolCreateInfo = {};
	poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolCreateInfo.maxSets = unifiedCount;
	poolCreateInfo.poolSizeCount = ARRAY_COUNT( poolSizes );
	poolCreateInfo.pPoolSizes = poolSizes;

	VK_CHECK( vkCreateDescriptorPool( renderObjects.device, &poolCreateInfo, NULL, &renderObjects.descriptorPool ) );
}

static void CreateSamplers() {
	VkSamplerCreateInfo samplerCreateInfo = {};
	samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
	samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
	samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
	VK_CHECK( vkCreateSampler( renderObjects.device, &samplerCreateInfo, NULL, &renderObjects.samplers[ SAMPLER_TYPE_LINEAR ] ) );
}

static void InitializeStagingBuffer() {
	const VkDeviceSize stagingSize = 128 * 1024 * 1024;
	VkBufferCreateInfo bufferCreateInfo = {};
	bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.size = stagingSize;
	VK_CHECK( vkCreateBuffer( renderObjects.device, &bufferCreateInfo, NULL, &stagingBuffer.buffer ) );

	VkMemoryRequirements memReq;
	vkGetBufferMemoryRequirements( renderObjects.device, stagingBuffer.buffer, &memReq );
	AllocateDeviceMemory( memReq, MEMORY_MAPPABLE, stagingBuffer.memory );
	VK_CHECK( vkBindBufferMemory( renderObjects.device, stagingBuffer.buffer, stagingBuffer.memory.memory, stagingBuffer.memory.offset ) );
	VK_CHECK( vkMapMemory( renderObjects.device, stagingBuffer.memory.memory, stagingBuffer.memory.offset, stagingSize, 0, &stagingBuffer.memoryData ) );
	stagingBuffer.currentOffset = 0;

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = renderObjects.commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;
	VK_CHECK( vkAllocateCommandBuffers( renderObjects.device, &commandBufferAllocateInfo, &stagingBuffer.commandBuffer ) );

	BeginStagingFrame();	// So we can stage resources during initialization
}

void Renderer_Init() {
	CreateInstance();

	GetPhysicalDevice();

	CreateDevice();

	extern void CreateSurface();
	CreateSurface();

	CreateSwapchain();

	CreateCommandBuffers();

	CreateSynchronizationPrimitives();

	InitializeStagingBuffer();

	CreateRenderTargets();

	CreateUnifiedPipelineLayout();

	CreateDescriptorPool();

	CreateSamplers();
}

void Renderer_BeginFrame() {
	extern void PumpMessages();
	PumpMessages();
	VK_CHECK( vkWaitForFences( renderObjects.device, 1, &renderObjects.renderFence, VK_TRUE, INFINITE ) );
	VK_CHECK( vkResetFences( renderObjects.device, 1, &renderObjects.renderFence ) );

	renderObjects.commandContext->Begin();
	BeginStagingFrame();
}

void Renderer_AcquireSwapchainImage() {
	VK_CHECK( vkAcquireNextImageKHR( renderObjects.device, renderObjects.swapchain, INFINITE, renderObjects.imageAcquireSemaphore, VK_NULL_HANDLE, &renderObjects.swapchainImageIndex ) );

	renderObjects.swapchainImage->SelectSwapchainImage( renderObjects.swapchainImageIndex );
}

void Renderer_EndFrame() {
	EndStagingFrame();
	renderObjects.commandContext->End();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
	VkCommandBuffer commandBuffer = renderObjects.commandContext->GetCommandBuffer();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &renderObjects.imageAcquireSemaphore;
	submitInfo.pWaitDstStageMask = &waitStageMask;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderObjects.renderCompleteSemaphore;
	VK_CHECK( vkQueueSubmit( renderObjects.queue, 1, &submitInfo, renderObjects.renderFence ) );

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &renderObjects.swapchain;
	presentInfo.pImageIndices = &renderObjects.swapchainImageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderObjects.renderCompleteSemaphore;
	VK_CHECK( vkQueuePresentKHR( renderObjects.queue, &presentInfo ) );
}