#include "Renderer.h"
#include "Image.h"
#include "CommandContext.h"
#include <vector>

renderObjects_t renderObjects;

#pragma comment( lib, "vulkan-1" )

static void CreateInstance() {
	std::vector< const char * > instanceExtensionNames = {
		VK_KHR_SURFACE_EXTENSION_NAME,
	};
	// Platform-specific code can be protected by an externally linked callback, so we'll do that here, since one of the instance layers is a platform surface extension.
	// Because this is only used in one place, it doesn't need to be declared in a header, just forward declared here.
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
	instanceCreateInfo.enabledLayerCount = ( uint32_t )instanceLayerNames.size();
	instanceCreateInfo.ppEnabledLayerNames = instanceLayerNames.data();
	instanceCreateInfo.enabledExtensionCount = ( uint32_t )instanceExtensionNames.size();
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNames.data();
	VK_CHECK( vkCreateInstance( &instanceCreateInfo, NULL, &renderObjects.instance ) );
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
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;	// We can expect the surface to support this capability
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// Same as above
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_FALSE;
	VK_CHECK( vkCreateSwapchainKHR( renderObjects.device, &swapchainCreateInfo, NULL, &renderObjects.swapchain ) );
	delete[] presentModes;
	delete[] surfaceFormats;
}

static void CreateCommandContexts() {
	// We still only need one command pool, since we're single-threaded (generally, multithreading and differing queue capabilities are the reasons for separate pools)
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.queueFamilyIndex = renderObjects.queueFamilyIndex;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

	VK_CHECK( vkCreateCommandPool( renderObjects.device, &commandPoolCreateInfo, NULL, &renderObjects.commandPool ) );

	renderObjects.commandContext = CommandContext::Create();
}

static void CreateSynchronizationPrimitives() {
	// We use a single render fence to reset our CommandContext.  We can have as many contexts as we want, and one fence will suffice, because it
	// covers the whole frame.  The main reason we would have another fence would be to have a ring buffer of contexts, one fence for each
	// frame potentially in flight.
	VkFenceCreateInfo fenceCreateInfo = {};
	fenceCreateInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceCreateInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;	// Using this flag lets the render loop avoid using different "first frame" logic for waiting on a fence
	VK_CHECK( vkCreateFence( renderObjects.device, &fenceCreateInfo, NULL, &renderObjects.renderFence ) );

	// The semaphores we have here are the boilerplate semaphores required by basically any Vulkan app.
	// One has to synchronize the swapchain image acquire to the submit that uses the image.
	// The other synchronizes the submit that fills out the swapchain image to the frame present.
	VkSemaphoreCreateInfo semaphoreCreateInfo = {};
	semaphoreCreateInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	VK_CHECK( vkCreateSemaphore( renderObjects.device, &semaphoreCreateInfo, NULL, &renderObjects.imageAcquireSemaphore ) );
	VK_CHECK( vkCreateSemaphore( renderObjects.device, &semaphoreCreateInfo, NULL, &renderObjects.renderCompleteSemaphore ) );
}

static void CreateRenderTargets() {
	// We introduce color and depth as proofs of concept on which to put our "real" rendering.  Color will be copied to swapchain later.
	renderObjects.colorImage = Image::Create( 1920, 1080, IMAGE_FORMAT_RGBA8, IMAGE_USAGE_RENDER_TARGET | IMAGE_USAGE_TRANSFER_SRC );
	renderObjects.depthImage = Image::Create( 1920, 1080, IMAGE_FORMAT_DEPTH, IMAGE_USAGE_RENDER_TARGET );
	renderObjects.swapchainImage = Image::CreateFromSwapchain();
}

static void CreateStubPipelineLayout() {
	// One pipeline layout for the renderer means we don't have to deal with dynamically changing requirements for shaders.
	// This Sprint doesn't have resources at all (still using shader logic only), so the layout can just be empty.
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	VkResult result = vkCreatePipelineLayout( renderObjects.device, &pipelineLayoutCreateInfo, NULL, &renderObjects.emptyLayout );
}

void Renderer_Init() {
	CreateInstance();

	GetPhysicalDevice();

	CreateDevice();

	extern void CreateSurface();	// Like the platform extension name, creating the surface is platform-specific, so we guard it in a separate file
	CreateSurface();

	CreateSwapchain();

	// Our essential design philosophy with these last functions is that although we can create arbitrary numbers of each thing, we have an engine-
	// defined set of resources from which we draw.  We hardcode the resources into the renderer to reduce complexity.
	CreateCommandContexts();

	CreateSynchronizationPrimitives();

	CreateRenderTargets();

	CreateStubPipelineLayout();
}

void Renderer_BeginFrame() {
	// Each frame, we wait for the fence (first frame, the wait is a NOP because it's signaled on creation), then reset it and start the command buffer.
	// The fence will be passed to the submit call so that it will be signaled when the buffer is finished executing.  Waiting for the fence here
	// ensures that the command context is finished (not in use) when we try to start recording to it again.
	VK_CHECK( vkWaitForFences( renderObjects.device, 1, &renderObjects.renderFence, VK_TRUE, VK_FOREVER ) );
	VK_CHECK( vkResetFences( renderObjects.device, 1, &renderObjects.renderFence ) );

	// Start recording.  Essentially, any calls on the context between BeginFrame and EndFrame will be executed when EndFrame is called
	renderObjects.commandContext->Begin();
}

void Renderer_AcquireSwapchainImage() {
	// Passing the acquire semaphore to be signaled when the swapchain image is ready.  It will be waited on by the submit
	VK_CHECK( vkAcquireNextImageKHR( renderObjects.device, renderObjects.swapchain, VK_FOREVER, renderObjects.imageAcquireSemaphore, VK_NULL_HANDLE, &renderObjects.swapchainImageIndex ) );

	// Switch the "active" image on the swapchain image to the one indicated by the index
	renderObjects.swapchainImage->SelectSwapchainImage( renderObjects.swapchainImageIndex );
}

void Renderer_EndFrame() {
	// Stop recording the context
	renderObjects.commandContext->End();

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkPipelineStageFlags waitStageMask = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
	VkCommandBuffer commandBuffer = renderObjects.commandContext->GetCommandBuffer();
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &renderObjects.imageAcquireSemaphore;	// Wait on the acquire
	submitInfo.pWaitDstStageMask = &waitStageMask;	// At the first command of the queue
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &renderObjects.renderCompleteSemaphore;	// Signal a semaphore to be waited on before the present occurs
	VK_CHECK( vkQueueSubmit( renderObjects.queue, 1, &submitInfo, renderObjects.renderFence ) );

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &renderObjects.swapchain;
	presentInfo.pImageIndices = &renderObjects.swapchainImageIndex;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &renderObjects.renderCompleteSemaphore;	// Wait for the rendering to be done before performing the present
	VK_CHECK( vkQueuePresentKHR( renderObjects.queue, &presentInfo ) );
}