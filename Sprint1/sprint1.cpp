#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>

#pragma comment( lib, "vulkan-1" )

#if defined( ARRAY_COUNT )
#undef ARRAY_COUNT
#endif
#define ARRAY_COUNT( x ) ( sizeof( ( x ) ) / sizeof( ( x )[ 0 ] ) )

const uint32_t SWAPCHAIN_IMAGE_COUNT = 2;

struct renderObjects_t {
	VkInstance			instance;
	VkPhysicalDevice	physicalDevice;
	uint32_t			queueFamilyIndex;
	VkDevice			device;
	VkQueue				queue;
	VkSurfaceKHR		surface;
	VkExtent2D			swapchainExtent;
	VkFormat			swapchainFormat;
	VkSwapchainKHR		swapchain;
	VkImage				swapchainImages[ SWAPCHAIN_IMAGE_COUNT ];
	VkImageView			swapchainImageViews[ SWAPCHAIN_IMAGE_COUNT ];
	VkFramebuffer		framebuffers[ SWAPCHAIN_IMAGE_COUNT ];
	VkCommandPool		commandPool;
	VkCommandBuffer		commandBuffer;
	VkRenderPass		renderPass;
	VkShaderModule		vertexModule;
	VkShaderModule		fragmentModule;
	VkPipelineLayout	pipelineLayout;
	VkPipeline			pipeline;
};

renderObjects_t barebonesRenderer;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd ) {
	char * className = "myClass";
	WNDCLASSEXA myClass = {};
	myClass.cbSize = sizeof( myClass );
	myClass.lpfnWndProc = DefWindowProcA;
	myClass.hInstance = hInstance;
	myClass.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	myClass.hCursor = LoadCursor( NULL, IDC_ARROW );
	myClass.lpszClassName = className;
	RegisterClassExA( &myClass );
	RECT rect = {};
	rect.right = 1600;
	rect.bottom = 900;
	DWORD windowStyle = WS_POPUP | WS_CAPTION | WS_VISIBLE;
	AdjustWindowRect( &rect, windowStyle, FALSE );
	uint32_t width = rect.right - rect.left;
	uint32_t height = rect.bottom - rect.top;
	HWND window = CreateWindowExA( 0, className, "My foist twiangow", windowStyle, rect.left, rect.top, width, height, NULL, NULL, hInstance, NULL );
	const char * instanceExtensionNames[] = {
		VK_KHR_SURFACE_EXTENSION_NAME,
		VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
	};
	const char * instanceLayerNames[] = {
#if !defined( NDEBUG )
		"VK_LAYER_LUNARG_standard_validation",
#endif
	};
	VkApplicationInfo appInfo = {};
	appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName = "Vulkan test app";
	appInfo.pEngineName = "Brute force engine";
	appInfo.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
	appInfo.engineVersion = VK_MAKE_VERSION( 0, 1, 0 );
	appInfo.apiVersion = VK_API_VERSION_1_0;
	VkInstanceCreateInfo instanceCreateInfo = {};
	instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instanceCreateInfo.enabledLayerCount = ARRAY_COUNT( instanceLayerNames );
	instanceCreateInfo.ppEnabledLayerNames = instanceLayerNames;
	instanceCreateInfo.enabledExtensionCount = ARRAY_COUNT( instanceExtensionNames );
	instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNames;
	VkResult result = vkCreateInstance( &instanceCreateInfo, NULL, &barebonesRenderer.instance );
	uint32_t physicalDeviceCount;
	result = vkEnumeratePhysicalDevices( barebonesRenderer.instance, &physicalDeviceCount, NULL );
	VkPhysicalDevice * allPhysicalDevices = new VkPhysicalDevice[ physicalDeviceCount ];
	result = vkEnumeratePhysicalDevices( barebonesRenderer.instance, &physicalDeviceCount, allPhysicalDevices );
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
			barebonesRenderer.physicalDevice = allPhysicalDevices[ i ];
			break;
		}
	}
	// We're not going to worry about not getting a physical device here.  It'll break if it's still NULL (the default value) when we use it
	delete[] allPhysicalDevices; // They're just handles.  This doesn't actually invalidate any of the physical devices
	uint32_t queueFamilyCount;
	vkGetPhysicalDeviceQueueFamilyProperties( barebonesRenderer.physicalDevice, &queueFamilyCount, NULL );
	VkQueueFamilyProperties * queueFamilyProperties = new VkQueueFamilyProperties[ queueFamilyCount ];
	vkGetPhysicalDeviceQueueFamilyProperties( barebonesRenderer.physicalDevice, &queueFamilyCount, queueFamilyProperties );
	barebonesRenderer.queueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
	VkQueueFlags desiredQueueCaps = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT;
	for ( uint32_t i = 0; i < queueFamilyCount; ++i ) {
		if ( ( queueFamilyProperties[ i ].queueFlags & desiredQueueCaps ) == desiredQueueCaps ) {
			barebonesRenderer.queueFamilyIndex = i;
			break;
		}
	}
	delete[] queueFamilyProperties;
	float queuePriority = 1.0f;	// This value must be between 0 and 1, by spec, but it's unimportant, because we'll only create the one
	VkDeviceQueueCreateInfo queueCreateInfo = {};
	queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queueCreateInfo.queueFamilyIndex = barebonesRenderer.queueFamilyIndex;
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
	result = vkCreateDevice( barebonesRenderer.physicalDevice, &deviceCreateInfo, NULL, &barebonesRenderer.device );
	vkGetDeviceQueue( barebonesRenderer.device, barebonesRenderer.queueFamilyIndex, 0, &barebonesRenderer.queue );
	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = hInstance;
	surfaceCreateInfo.hwnd = window;
	result = vkCreateWin32SurfaceKHR( barebonesRenderer.instance, &surfaceCreateInfo, NULL, &barebonesRenderer.surface );
	VkBool32 presentSupported;
	vkGetPhysicalDeviceSurfaceSupportKHR( barebonesRenderer.physicalDevice, barebonesRenderer.queueFamilyIndex, barebonesRenderer.surface, &presentSupported );
	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( barebonesRenderer.physicalDevice, barebonesRenderer.surface, &surfaceCapabilities );
	barebonesRenderer.swapchainExtent = surfaceCapabilities.currentExtent;
	uint32_t surfaceFormatCount;
	result = vkGetPhysicalDeviceSurfaceFormatsKHR( barebonesRenderer.physicalDevice, barebonesRenderer.surface, &surfaceFormatCount, NULL );
	VkSurfaceFormatKHR * surfaceFormats = new VkSurfaceFormatKHR[ surfaceFormatCount ];
	result = vkGetPhysicalDeviceSurfaceFormatsKHR( barebonesRenderer.physicalDevice, barebonesRenderer.surface, &surfaceFormatCount, surfaceFormats );
	uint32_t presentModeCount;
	result = vkGetPhysicalDeviceSurfacePresentModesKHR( barebonesRenderer.physicalDevice, barebonesRenderer.surface, &presentModeCount, NULL );
	VkPresentModeKHR * presentModes = new VkPresentModeKHR[ presentModeCount ];
	result = vkGetPhysicalDeviceSurfacePresentModesKHR( barebonesRenderer.physicalDevice, barebonesRenderer.surface, &presentModeCount, presentModes );
	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;	// This is required to be supported, so we'll use it as a fallback
	for ( uint32_t i = 0; i < presentModeCount; ++i ) {
		if ( presentModes[ i ] == VK_PRESENT_MODE_FIFO_RELAXED_KHR ) {
			presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
			break;
		}
	}
	barebonesRenderer.swapchainFormat = surfaceFormats[ 0 ].format;
	VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
	swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchainCreateInfo.surface = barebonesRenderer.surface;
	swapchainCreateInfo.minImageCount = SWAPCHAIN_IMAGE_COUNT;	// Technically, we should check this against the surface capabilities, but we're probably fine with a sane number like 2 or 3
	swapchainCreateInfo.imageFormat = surfaceFormats[ 0 ].format;
	swapchainCreateInfo.imageColorSpace = surfaceFormats[ 0 ].colorSpace;	// These are truly unimportant.  What you'll normally get is an SRGB nonlinear color space, and a BGRA8 format.  These are fine
	swapchainCreateInfo.imageExtent = barebonesRenderer.swapchainExtent;
	swapchainCreateInfo.imageArrayLayers = 1;
	swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;	// We can expect the surface to support this capability
	swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// Same as above
	swapchainCreateInfo.presentMode = presentMode;
	swapchainCreateInfo.clipped = VK_FALSE;
	result = vkCreateSwapchainKHR( barebonesRenderer.device, &swapchainCreateInfo, NULL, &barebonesRenderer.swapchain );
	delete[] presentModes;
	delete[] surfaceFormats;
	uint32_t swapchainImageCount;
	result = vkGetSwapchainImagesKHR( barebonesRenderer.device, barebonesRenderer.swapchain, &swapchainImageCount, NULL );
	// swapchainImageCount should be SWAPCHAIN_IMAGE_COUNT; we're just gonna expect it
	result = vkGetSwapchainImagesKHR( barebonesRenderer.device, barebonesRenderer.swapchain, &swapchainImageCount, barebonesRenderer.swapchainImages );
	VkCommandPoolCreateInfo commandPoolCreateInfo = {};
	commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	commandPoolCreateInfo.queueFamilyIndex = barebonesRenderer.queueFamilyIndex;
	result = vkCreateCommandPool( barebonesRenderer.device, &commandPoolCreateInfo, NULL, &barebonesRenderer.commandPool );
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
	commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	commandBufferAllocateInfo.commandPool = barebonesRenderer.commandPool;
	commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	commandBufferAllocateInfo.commandBufferCount = 1;
	result = vkAllocateCommandBuffers( barebonesRenderer.device, &commandBufferAllocateInfo, &barebonesRenderer.commandBuffer );
	VkImageViewCreateInfo imageViewCreateInfo = {};
	imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	imageViewCreateInfo.format = barebonesRenderer.swapchainFormat;
	imageViewCreateInfo.components = {
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
		VK_COMPONENT_SWIZZLE_IDENTITY,
	};
	imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
	imageViewCreateInfo.subresourceRange.levelCount = 1;
	imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
	imageViewCreateInfo.subresourceRange.layerCount = 1;
	for ( uint32_t i = 0; i < SWAPCHAIN_IMAGE_COUNT; ++i ) {
		imageViewCreateInfo.image = barebonesRenderer.swapchainImages[ i ];
		result = vkCreateImageView( barebonesRenderer.device, &imageViewCreateInfo, NULL, &barebonesRenderer.swapchainImageViews[ i ] );
	}
	VkAttachmentDescription attachmentDescription = {};
	attachmentDescription.format = barebonesRenderer.swapchainFormat;
	attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;
	attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	VkAttachmentReference colorAttachmentReference = {};
	colorAttachmentReference.attachment = 0;
	colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
	VkSubpassDescription subpassDescription = {};
	subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpassDescription.colorAttachmentCount = 1;
	subpassDescription.pColorAttachments = &colorAttachmentReference;
	VkRenderPassCreateInfo renderPassCreateInfo = {};
	renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassCreateInfo.attachmentCount = 1;
	renderPassCreateInfo.pAttachments = &attachmentDescription;
	renderPassCreateInfo.subpassCount = 1;
	renderPassCreateInfo.pSubpasses = &subpassDescription;
	result = vkCreateRenderPass( barebonesRenderer.device, &renderPassCreateInfo, NULL, &barebonesRenderer.renderPass );
	VkFramebufferCreateInfo framebufferCreateInfo = {};
	framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
	framebufferCreateInfo.renderPass = barebonesRenderer.renderPass;
	framebufferCreateInfo.attachmentCount = 1;
	framebufferCreateInfo.width = barebonesRenderer.swapchainExtent.width;
	framebufferCreateInfo.height = barebonesRenderer.swapchainExtent.height;
	framebufferCreateInfo.layers = 1;
	for ( uint32_t i = 0; i < SWAPCHAIN_IMAGE_COUNT; ++i ) {
		framebufferCreateInfo.pAttachments = &barebonesRenderer.swapchainImageViews[ i ];
		result = vkCreateFramebuffer( barebonesRenderer.device, &framebufferCreateInfo, NULL, &barebonesRenderer.framebuffers[ i ] );
	}
	HANDLE shaderHandle;
	shaderHandle = CreateFileA( "simpleTri.vspv", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	DWORD fileSize;
	fileSize = GetFileSize( shaderHandle, NULL );
	char * fileBuffer = new char[ fileSize ];
	DWORD bytesRead;
	ReadFile( shaderHandle, fileBuffer, fileSize, &bytesRead, NULL );
	VkShaderModuleCreateInfo shaderModuleCreateInfo = {};
	shaderModuleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	shaderModuleCreateInfo.codeSize = fileSize;
	shaderModuleCreateInfo.pCode = ( uint32_t * )fileBuffer;
	result = vkCreateShaderModule( barebonesRenderer.device, &shaderModuleCreateInfo, NULL, &barebonesRenderer.vertexModule );
	delete[] fileBuffer;
	CloseHandle( shaderHandle );
	shaderHandle = CreateFileA( "simpleTri.fspv", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
	fileSize = GetFileSize( shaderHandle, NULL );
	fileBuffer = new char[ fileSize ];
	ReadFile( shaderHandle, fileBuffer, fileSize, &bytesRead, NULL );
	shaderModuleCreateInfo.codeSize = fileSize;
	shaderModuleCreateInfo.pCode = ( uint32_t * )fileBuffer;
	result = vkCreateShaderModule( barebonesRenderer.device, &shaderModuleCreateInfo, NULL, &barebonesRenderer.fragmentModule );
	VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
	pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	result = vkCreatePipelineLayout( barebonesRenderer.device, &pipelineLayoutCreateInfo, NULL, &barebonesRenderer.pipelineLayout );
	VkPipelineShaderStageCreateInfo stages[ 2 ] = {};
	stages[ 0 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[ 0 ].stage = VK_SHADER_STAGE_VERTEX_BIT;
	stages[ 0 ].module = barebonesRenderer.vertexModule;
	stages[ 0 ].pName = "main";
	stages[ 1 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stages[ 1 ].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	stages[ 1 ].module = barebonesRenderer.fragmentModule;
	stages[ 1 ].pName = "main";
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
	vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
	inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;
	VkViewport viewport = {};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = ( float )barebonesRenderer.swapchainExtent.width;
	viewport.height = ( float )barebonesRenderer.swapchainExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	VkRect2D scissor = {};
	scissor.offset.x = 0;
	scissor.offset.y = 0;
	scissor.extent.width = barebonesRenderer.swapchainExtent.width;
	scissor.extent.height = barebonesRenderer.swapchainExtent.height;
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {};
	viewportStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportStateCreateInfo.viewportCount = 1;
	viewportStateCreateInfo.pViewports = &viewport;
	viewportStateCreateInfo.scissorCount = 1;
	viewportStateCreateInfo.pScissors = &scissor;
	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
	rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizationStateCreateInfo.depthClampEnable;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;
	rasterizationStateCreateInfo.lineWidth = 1.0f;
	VkSampleMask sampleMask = 0xFFFFFFFF;
	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
	multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	multisampleStateCreateInfo.minSampleShading = 1.0f;	// Required by spec without an optional feature enabled
	multisampleStateCreateInfo.pSampleMask = &sampleMask;
	multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
	colorBlendAttachmentState.blendEnable = VK_FALSE;
	colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
	colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;
	colorBlendStateCreateInfo.attachmentCount = 1;
	colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {};
	graphicsPipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	graphicsPipelineCreateInfo.stageCount = 2;
	graphicsPipelineCreateInfo.pStages = stages;
	graphicsPipelineCreateInfo.pVertexInputState = &vertexInputStateCreateInfo;
	graphicsPipelineCreateInfo.pInputAssemblyState = &inputAssemblyStateCreateInfo;
	graphicsPipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
	graphicsPipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
	graphicsPipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
	graphicsPipelineCreateInfo.pColorBlendState = &colorBlendStateCreateInfo;
	graphicsPipelineCreateInfo.layout = barebonesRenderer.pipelineLayout;
	graphicsPipelineCreateInfo.renderPass = barebonesRenderer.renderPass;
	graphicsPipelineCreateInfo.subpass = 0;
	result = vkCreateGraphicsPipelines( barebonesRenderer.device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, NULL, &barebonesRenderer.pipeline );
	while ( true ) {
		// THE RENDER LOOP!!!!!!!!
		uint32_t imageIndex;
		result = vkAcquireNextImageKHR( barebonesRenderer.device, barebonesRenderer.swapchain, INFINITE, VK_NULL_HANDLE, VK_NULL_HANDLE, &imageIndex );
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		result = vkBeginCommandBuffer( barebonesRenderer.commandBuffer, &commandBufferBeginInfo );
		VkClearValue clearColor = {};
		clearColor.color.float32[ 0 ] = 1.0f;
		clearColor.color.float32[ 1 ] = 1.0f;
		clearColor.color.float32[ 2 ] = 1.0f;
		clearColor.color.float32[ 3 ] = 1.0f;
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = barebonesRenderer.renderPass;
		renderPassBeginInfo.framebuffer = barebonesRenderer.framebuffers[ imageIndex ];
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = barebonesRenderer.swapchainExtent;
		renderPassBeginInfo.clearValueCount = 1;
		renderPassBeginInfo.pClearValues = &clearColor;
		vkCmdBeginRenderPass( barebonesRenderer.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
		vkCmdBindPipeline( barebonesRenderer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, barebonesRenderer.pipeline );
		vkCmdDraw( barebonesRenderer.commandBuffer, 3, 1, 0, 0 );
		vkCmdEndRenderPass( barebonesRenderer.commandBuffer );
		result = vkEndCommandBuffer( barebonesRenderer.commandBuffer );
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &barebonesRenderer.commandBuffer;
		result = vkQueueSubmit( barebonesRenderer.queue, 1, &submitInfo, VK_NULL_HANDLE );
		result = vkQueueWaitIdle( barebonesRenderer.queue );
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &barebonesRenderer.swapchain;
		presentInfo.pImageIndices = &imageIndex;
		result = vkQueuePresentKHR( barebonesRenderer.queue, &presentInfo );
	}
}