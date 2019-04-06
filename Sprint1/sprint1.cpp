#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan.h>
#include <vector>

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

// We store all of our important Vulkan objects on this global singleton container.  Here, they are easily accessed from anywhere (we'll use this paradigm going forward)
renderObjects_t barebonesRenderer;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd ) {
	HWND window;
	{
		// Boilerplate hardcoded window
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
		window = CreateWindowExA( 0, className, "My foist twiangow", windowStyle, rect.left, rect.top, width, height, NULL, NULL, hInstance, NULL );
	}

	{
	// Some extensions are required to get anything on screen.  As far as the instance is concerned, we have
	// to have the generic surface extension, as well as the platform-specific surface extension, in this case, Win32
		const char * instanceExtensionNames[] = {
			VK_KHR_SURFACE_EXTENSION_NAME,
			VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
		};
		// The standard validation layer should automatically spit out errors to the debug output.
		// We don't bother doing so here, but it is generally good practice to eliminate any errors it spits out.
		std::vector< const char * > instanceLayerNames = {
	#if defined( _DEBUG )
			"VK_LAYER_LUNARG_standard_validation",
	#endif
		};
		uint32_t propCount;
		vkEnumerateInstanceLayerProperties( &propCount, NULL );
		VkLayerProperties * props = new VkLayerProperties[ propCount ];
		vkEnumerateInstanceLayerProperties( &propCount, props );

		VkApplicationInfo appInfo = {};	// We start each struct with "= {}" to zero-initialize all bytes.  We could also do a memset here, but "= {}" is more concise.
		// sType, common to almost all "info" structs, identifies the structure using an enum value.
		// The spec requires the value corresponding to the struct used to be set in all cases where the member is present.
		// pNext is a pointer to another struct, used only in cases where an extension's functionality requires more information
		// than the core struct provides.  pNext must be NULL if no extended information is required (i.e. in most cases).
		// sType and pNext are the first two members of almost every struct, in that order, because in the case
		// of an extension struct, pNext can be cast to any struct that a driver searches for, and then sType can be used
		// to verify what the provided struct is (since pNext can point to anything)
		appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		appInfo.pNext = NULL;
		appInfo.pApplicationName = "Vulkan test app";
		appInfo.pEngineName = "Brute force engine";
		appInfo.applicationVersion = VK_MAKE_VERSION( 0, 1, 0 );
		appInfo.engineVersion = VK_MAKE_VERSION( 0, 1, 0 );
		appInfo.apiVersion = VK_API_VERSION_1_0;
		VkInstanceCreateInfo instanceCreateInfo = {};
		instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		instanceCreateInfo.enabledLayerCount = ( uint32_t )instanceLayerNames.size();
		instanceCreateInfo.ppEnabledLayerNames = instanceLayerNames.data();
		instanceCreateInfo.enabledExtensionCount = ARRAY_COUNT( instanceExtensionNames );
		instanceCreateInfo.ppEnabledExtensionNames = instanceExtensionNames;
		// The instance is the top-level object that serves as the entrypoint to the API.  It is the only object that
		// does not require a previously existing object as a dispatcher.
		VkResult result = vkCreateInstance( &instanceCreateInfo, NULL, &barebonesRenderer.instance );
	}

	{
	// We must select a physical device (essentially a GPU) as the backing hardware for a logical device on which
	// to run commands, create resources, and allocate memory
		uint32_t physicalDeviceCount;
		VkResult result = vkEnumeratePhysicalDevices( barebonesRenderer.instance, &physicalDeviceCount, NULL );
		VkPhysicalDevice * allPhysicalDevices = new VkPhysicalDevice[ physicalDeviceCount ];
		result = vkEnumeratePhysicalDevices( barebonesRenderer.instance, &physicalDeviceCount, allPhysicalDevices );
		// We could look for a device type that is "DISCRETE", but instead, we check for AMD and NVIDIA explicitly.
		// This means that a box without a discrete GPU (or with a non-AMD, non-NVIDIA discrete GPU) is going to fail at running this sample.
		// In such a case, modify this code to pick the first physical device instead of being so picky.
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
				barebonesRenderer.physicalDevice = allPhysicalDevices[ i ];
				break;
			}
		}
		// We're not going to worry about not getting a physical device here.  It'll break if it's still NULL (the default value) when we use it
		delete[] allPhysicalDevices; // They're just handles.  This doesn't actually invalidate any of the physical devices
	}

	{
	// We need a graphics/compute/transfer queue.  To get this queue, we need to find the family that has the proper capabilities
	// and add a VkDeviceQueueCreateInfo that creates a queue from this family to the VkDeviceCreateInfo.
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
		// We need to activate the swapchain extension in order to use a swapchain - the only object that allows rendering to a surface
		const char * deviceExtensionNames[] = {
			VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		};
		VkDeviceCreateInfo deviceCreateInfo = {};
		deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		deviceCreateInfo.queueCreateInfoCount = 1;
		deviceCreateInfo.pQueueCreateInfos = &queueCreateInfo;
		deviceCreateInfo.enabledExtensionCount = ARRAY_COUNT( deviceExtensionNames );
		deviceCreateInfo.ppEnabledExtensionNames = deviceExtensionNames;
		// The device enables most of our calls to create resources (like buffers, images, command pools, etc.).
		VkResult result = vkCreateDevice( barebonesRenderer.physicalDevice, &deviceCreateInfo, NULL, &barebonesRenderer.device );
		// But the queue is the "processor" that allows us to submit GPU commands (through command buffers).
		vkGetDeviceQueue( barebonesRenderer.device, barebonesRenderer.queueFamilyIndex, 0, &barebonesRenderer.queue );
	}

	{
		VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
		surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
		surfaceCreateInfo.hinstance = hInstance;
		surfaceCreateInfo.hwnd = window;
		// The "create surface" call is platform-specific and requires an extension.  In this case, VK_KHR_WIN32_SURFACE_EXTENSION_NAME.
		VkResult result = vkCreateWin32SurfaceKHR( barebonesRenderer.instance, &surfaceCreateInfo, NULL, &barebonesRenderer.surface );
		VkBool32 presentSupported;
		// Presentation support must be queried using this call.  This is a good candidate for assertion.
		vkGetPhysicalDeviceSurfaceSupportKHR( barebonesRenderer.physicalDevice, barebonesRenderer.queueFamilyIndex, barebonesRenderer.surface, &presentSupported );
		VkSurfaceCapabilitiesKHR surfaceCapabilities;
		// Querying for surface capabilities is required to choose valid options for the swapchain
		result = vkGetPhysicalDeviceSurfaceCapabilitiesKHR( barebonesRenderer.physicalDevice, barebonesRenderer.surface, &surfaceCapabilities );
		// The swapchain should, in normal use, match the extent of the window (resizing the window invalidates the surface, which requires
		// recreating both it and the swapchain.
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
		// The present mode we want is FIFO_RELAXED, which uses v-sync if the frame is fast enough, or presents immediately with
		// tearing if the frame misses the vertical blank.  It's preferable for keeping the video relatively close to the monitor's refresh rate.
		for ( uint32_t i = 0; i < presentModeCount; ++i ) {
			if ( presentModes[ i ] == VK_PRESENT_MODE_FIFO_RELAXED_KHR ) {
				presentMode = VK_PRESENT_MODE_FIFO_RELAXED_KHR;
				break;
			}
		}
		barebonesRenderer.swapchainFormat = surfaceFormats[ 0 ].format;	// It's not terribly important what the format is, just that we know it so it can be used later
		VkSwapchainCreateInfoKHR swapchainCreateInfo = {};
		swapchainCreateInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		swapchainCreateInfo.surface = barebonesRenderer.surface;
		swapchainCreateInfo.minImageCount = SWAPCHAIN_IMAGE_COUNT;	// Technically, we should check this against the surface capabilities, but we're probably fine with a sane number like 2 or 3
		swapchainCreateInfo.imageFormat = surfaceFormats[ 0 ].format;
		swapchainCreateInfo.imageColorSpace = surfaceFormats[ 0 ].colorSpace;	// These are truly unimportant.  What you'll normally get is an SRGB nonlinear color space, and a BGRA8 format.  These are fine
		swapchainCreateInfo.imageExtent = barebonesRenderer.swapchainExtent;
		swapchainCreateInfo.imageArrayLayers = 1;
		// Usage is very important.  If the image is used in a way that is not supported by the supplied flags, the driver may just fail the operation.
		// Validation might catch this, but it might not, so if you have a bug that, for all reason, is not working, check the usage flags.
		swapchainCreateInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		// We'll always set the sharing mode to EXCLUSIVE.  This means that no resources will be used concurrently on multiple queues.
		// It also simplifies any structs that take the sharing mode, as they will also require compatible queue family indices if the sharing mode
		// is not EXCLUSIVE.  This way, we can leave all three values (sharing mode, index count, and index array pointer) to 0 bytes, as EXCLUSIVE
		// is the first enum value, and a 0 count and NULL pointer also happen to be 0s.
		swapchainCreateInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchainCreateInfo.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;	// We can expect the surface to support this capability, which does nothing to the image
		swapchainCreateInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;	// Same as above, just means that the window is opaque
		swapchainCreateInfo.presentMode = presentMode;
		// This value being false means that we own all rendered fragments of the window, even if it's partially obscured.
		// We do this because there is potential, even if slight, to have graphical bugs caused by failure to perform side-effects
		// in a fragment shader for occluded fragments of the window.  Just a precaution against truly insidious potential bugs.
		swapchainCreateInfo.clipped = VK_FALSE;
		result = vkCreateSwapchainKHR( barebonesRenderer.device, &swapchainCreateInfo, NULL, &barebonesRenderer.swapchain );
		delete[] presentModes;
		delete[] surfaceFormats;
		uint32_t swapchainImageCount;
		// The way to render to anything is through a VkImage, so we have to retrieve VkImages that were created implicitly
		// when we created the swapchain.
		result = vkGetSwapchainImagesKHR( barebonesRenderer.device, barebonesRenderer.swapchain, &swapchainImageCount, NULL );
		// swapchainImageCount should be SWAPCHAIN_IMAGE_COUNT; we're just gonna expect it
		result = vkGetSwapchainImagesKHR( barebonesRenderer.device, barebonesRenderer.swapchain, &swapchainImageCount, barebonesRenderer.swapchainImages );
	}

	{
		// Commands that perform actual work (compute or rendering) on the GPU are exclusively recorded to command buffers
		// and executed on a queue.  Command buffers can only be allocated from a command pool, so we create a global one.
		VkCommandPoolCreateInfo commandPoolCreateInfo = {};
		commandPoolCreateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		// This flag allows us to reuse command buffers.  Without it, we'd have to free and allocate them upon reuse.
		commandPoolCreateInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		// Supplying the queue family index here means we can only submit command buffers allocated from this pool to a queue from this family.
		// It also implies the kinds of commands that can be recorded to buffers allocated from this pool, based on the queue capabilities.
		commandPoolCreateInfo.queueFamilyIndex = barebonesRenderer.queueFamilyIndex;
		VkResult result = vkCreateCommandPool( barebonesRenderer.device, &commandPoolCreateInfo, NULL, &barebonesRenderer.commandPool );
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {};
		commandBufferAllocateInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		commandBufferAllocateInfo.commandPool = barebonesRenderer.commandPool;
		// Primary command buffers can be submitted to a queue.  Secondary command buffers can only be executed by a primary command buffer.
		// We have no use for the latter, so we specify the former.
		commandBufferAllocateInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		commandBufferAllocateInfo.commandBufferCount = 1;
		result = vkAllocateCommandBuffers( barebonesRenderer.device, &commandBufferAllocateInfo, &barebonesRenderer.commandBuffer );
	}

	{
		// Rendering and compute work cannot operate on raw VkImages (transfers can, though); they must go through a VkImageView.
		// This view can be comprised of a change in format, a swizzle, and/or a specific subresource (aspect, mips, layers).
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
		imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;	// Color, depth, or stencil
		imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
		imageViewCreateInfo.subresourceRange.levelCount = 1;	// VK_REMAINING_MIP_LEVELS will view all mips starting with baseMipLevel
		imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
		imageViewCreateInfo.subresourceRange.layerCount = 1;	// VK_REMAINING_ARRAY_LAYERS will view all layers starting with baseArrayLayer
		// We create a view for each swapchain image (each view is bound irrevocably to one image), and each frame will cycle through the swapchain images.
		for ( uint32_t i = 0; i < SWAPCHAIN_IMAGE_COUNT; ++i ) {
			imageViewCreateInfo.image = barebonesRenderer.swapchainImages[ i ];
			VkResult result = vkCreateImageView( barebonesRenderer.device, &imageViewCreateInfo, NULL, &barebonesRenderer.swapchainImageViews[ i ] );
		}
	}

	{
		// Render passes describe the attachments that will be bound at a given time: including their formats,
		// order, what to do to each as the pass begins and ends (discard the contents, load the contents, clear the image)
		// and what layouts the attachments should be in at different stages.
		// Render passes can further be subdivided into subpasses, each of which can change the layout of attachments or prepare them
		// for different kinds of usage (color or depth-stencil targets, resolving multisampled images, per-fragment input to shaders).

		// I personally think that most of this is unnecessary fluff, but we have to use render passes to draw anything,
		// so we'll use the fewest features we can.  This means color attachments and depth stencil attachments only,
		// load and clear only, store only, one subpass, and a small subset of layouts.
		VkAttachmentDescription attachmentDescription = {};
		attachmentDescription.format = barebonesRenderer.swapchainFormat;
		attachmentDescription.samples = VK_SAMPLE_COUNT_1_BIT;	// No multisampling
		attachmentDescription.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;	// Clear the attachment upon beginning the pass
		attachmentDescription.storeOp = VK_ATTACHMENT_STORE_OP_STORE;	// Store the contents upon ending the pass
		attachmentDescription.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;	// Not necessary for a color attachment
		attachmentDescription.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachmentDescription.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// When beginning the pass, it will be transferred to this layout (optimal for rendering)
		attachmentDescription.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;	// When ending the pass, it will be transferred to this layout (able to be presented to screen)
		VkAttachmentReference colorAttachmentReference = {};
		colorAttachmentReference.attachment = 0;	// The attachment will be the first in the framebuffer
		colorAttachmentReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;	// This is the layout for the first and only subpass
		VkSubpassDescription subpassDescription = {};
		subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;	// Implicitly required
		subpassDescription.colorAttachmentCount = 1;
		subpassDescription.pColorAttachments = &colorAttachmentReference;
		VkRenderPassCreateInfo renderPassCreateInfo = {};
		renderPassCreateInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassCreateInfo.attachmentCount = 1;
		renderPassCreateInfo.pAttachments = &attachmentDescription;
		renderPassCreateInfo.subpassCount = 1;
		renderPassCreateInfo.pSubpasses = &subpassDescription;
		VkResult result = vkCreateRenderPass( barebonesRenderer.device, &renderPassCreateInfo, NULL, &barebonesRenderer.renderPass );
	}

	{
		// A framebuffer contains all of the actual image views that will be bound at one time to the execution of a render pass.
		VkFramebufferCreateInfo framebufferCreateInfo = {};
		framebufferCreateInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferCreateInfo.renderPass = barebonesRenderer.renderPass;
		framebufferCreateInfo.attachmentCount = 1;
		framebufferCreateInfo.width = barebonesRenderer.swapchainExtent.width;
		framebufferCreateInfo.height = barebonesRenderer.swapchainExtent.height;
		framebufferCreateInfo.layers = 1;
		// Because there is a single image view per swapchain image, we need a separate framebuffer for each swapchain image as well.
		for ( uint32_t i = 0; i < SWAPCHAIN_IMAGE_COUNT; ++i ) {
			framebufferCreateInfo.pAttachments = &barebonesRenderer.swapchainImageViews[ i ];
			VkResult result = vkCreateFramebuffer( barebonesRenderer.device, &framebufferCreateInfo, NULL, &barebonesRenderer.framebuffers[ i ] );
		}
	}

	{
		// Shader modules are the objects that hold shader binary code (in SPIR-V form).  We just read in previously compiled SPIR-V code to make our modules.
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
		VkResult result = vkCreateShaderModule( barebonesRenderer.device, &shaderModuleCreateInfo, NULL, &barebonesRenderer.vertexModule );
		delete[] fileBuffer;
		CloseHandle( shaderHandle );
		shaderHandle = CreateFileA( "simpleTri.fspv", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL );
		fileSize = GetFileSize( shaderHandle, NULL );
		fileBuffer = new char[ fileSize ];
		ReadFile( shaderHandle, fileBuffer, fileSize, &bytesRead, NULL );
		shaderModuleCreateInfo.codeSize = fileSize;
		shaderModuleCreateInfo.pCode = ( uint32_t * )fileBuffer;
		result = vkCreateShaderModule( barebonesRenderer.device, &shaderModuleCreateInfo, NULL, &barebonesRenderer.fragmentModule );
		delete[] fileBuffer;
		CloseHandle( shaderHandle );
	}

	{
		// A pipeline layout describes all descriptor sets and push-constants (data recorded directly onto a command buffer for use in a shader)
		// to be used in a single draw or dispatch.  Our draw requires no resources (it's all in the shader), so the layout will just be empty.
		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.pushConstantRangeCount = 0;
		pipelineLayoutCreateInfo.pPushConstantRanges = NULL;
		pipelineLayoutCreateInfo.setLayoutCount = 0;
		pipelineLayoutCreateInfo.pSetLayouts = NULL;
		VkResult result = vkCreatePipelineLayout( barebonesRenderer.device, &pipelineLayoutCreateInfo, NULL, &barebonesRenderer.pipelineLayout );
	}

	{
		// The pipeline takes all of the shaders to be bound at once.  This is usually just the two (vertex and fragment), but can optionally include
		// geometry and tessellation shaders.
		VkPipelineShaderStageCreateInfo stages[ 2 ] = {};
		stages[ 0 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[ 0 ].stage = VK_SHADER_STAGE_VERTEX_BIT;	// Specify the stage
		stages[ 0 ].module = barebonesRenderer.vertexModule;
		stages[ 0 ].pName = "main";	// The name of the shader entrypoint
		stages[ 1 ].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		stages[ 1 ].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		stages[ 1 ].module = barebonesRenderer.fragmentModule;
		stages[ 1 ].pName = "main";
		// The vertex input state describes all vertex buffer bindings and vertex attributes.  We don't use any now, because the shader has data hardcoded into it.
		VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {};
		vertexInputStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		// The input assembly state translates the draw calls (raw vertex buffer or index buffers) into primitives.
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {};
		inputAssemblyStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateCreateInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;	// Raw triangle list
		inputAssemblyStateCreateInfo.primitiveRestartEnable = VK_FALSE;	// Only needed for strips and fans.  Allows a word of all 1s to cut the strip or fan and start a new one.
		// The viewport state controls the viewport transformation.  It expects both a viewport and a scissor.
		VkViewport viewport = {};
		// The viewport area origin (upper-left corner) is denoted by the x and y members here.  (0,0) denotes the true upper-left corner of the attachment.
		// It's possible to specify a y-up viewport (which lets you forego inverting the y-coordinate in the shader) by giving a negative height, but in the new coordinate system,
		// the true origin becomes the bottom left, and the viewport origin must still be the top-left of the viewport, so giving a full viewport in that case would be (0,height).
		// In this case, though, we'll handle the y flip in the shader manually (the negative height trick requires an extension) to simplify the CPU code and keep the problem
		// in sight (we want to stay aware of the way the viewport transformation is done).
		viewport.x = 0.0f;
		viewport.y = 0.0f;
		viewport.width = ( float )barebonesRenderer.swapchainExtent.width;
		viewport.height = ( float )barebonesRenderer.swapchainExtent.height;
		viewport.minDepth = 0.0f;
		viewport.maxDepth = 1.0f;
		// The scissor works basically the same way as the viewport (the origin is the upper-left of the scissor).
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
		// Rasterization state controls how primitives are transformed into fragments
		VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {};
		rasterizationStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;	// Used for depth trickery
		rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;	// Setting this to true will effectively disable rendering after the vertex shader (no output)
		rasterizationStateCreateInfo.polygonMode = VK_POLYGON_MODE_FILL;	// We can fully rasterize the polygon or just draw lines (wireframe)
		rasterizationStateCreateInfo.cullMode = VK_CULL_MODE_BACK_BIT;	// Choose whether to cull front faces, back faces, both, or neither
		rasterizationStateCreateInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;	// Select the winding order for culling
		rasterizationStateCreateInfo.depthBiasEnable = VK_FALSE;	// Used for more depth trickery (z-fighting, chiefly)
		rasterizationStateCreateInfo.lineWidth = 1.0f;	// Width of lines (wireframe mode, or line primitive topology); 1 is the default and required unless a specific feature is enabled
		// For multisampled images, this mask specifies which samples to take.  We set this to all 1s to force all samples to be rasterized.  This is still required for a single sample.
		VkSampleMask sampleMask = 0xFFFFFFFF;
		// Multisampled rendering is a core feature of Vulkan, so even for no multisampling, we are required to specify that there will be exactly one sample, and how to treat it.
		VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {};
		multisampleStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCreateInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;	// One sample
		multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;	// Not needed for one sample
		multisampleStateCreateInfo.minSampleShading = 1.0f;	// Required by spec without an optional feature enabled
		multisampleStateCreateInfo.pSampleMask = &sampleMask;
		multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;	// Not needed
		multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;	// Not needed
		// Color blend state controls how fragment shader output blends with what's already in the attachment.
		// Since multiple attachments can be bound, there must be an attachment state for each, but we'll only use one color attachment, so we'll never need more than one.
		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.blendEnable = VK_FALSE;	// With blending disabled, none of the omitted members matter; the fragment output will overwrite what's in the attachment
		// The colorWriteMask determines which channels can be written after blending.  By default, we just enable all channels.
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {};
		colorBlendStateCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCreateInfo.logicOpEnable = VK_FALSE;	// Logic ops are more advanced blending.  We won't use them
		colorBlendStateCreateInfo.attachmentCount = 1;
		colorBlendStateCreateInfo.pAttachments = &colorBlendAttachmentState;
		// The graphics pipeline create info mostly just aggregates the create infos defined above.
		// It compiles all of the state used to draw into a single object that can be bound in a single call.
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
		graphicsPipelineCreateInfo.layout = barebonesRenderer.pipelineLayout;	// The resource layout to be used in the draw
		// A render pass compatible with the one that will be active when the pipeline is bound; check the spec for rules, but essentially,
		// if the number and types of attachments match, two render passes are compatible
		graphicsPipelineCreateInfo.renderPass = barebonesRenderer.renderPass;
		graphicsPipelineCreateInfo.subpass = 0;	// One reason we use only one subpass is that using more could easily compound pipeline permutations
		// The pipeline cache can be non-existent, which simply means that no cache will be used.  It doesn't affect performance noticeably at this small scale,
		// and I would need to run tests to see if it would affect performance at a larger scale.
		VkResult result = vkCreateGraphicsPipelines( barebonesRenderer.device, VK_NULL_HANDLE, 1, &graphicsPipelineCreateInfo, NULL, &barebonesRenderer.pipeline );
	}

	while ( true ) {
		// THE RENDER LOOP!!!!!!!!
		uint32_t imageIndex;
		// The next image of the swapchain must be acquired before we can render to it.  This synchronizes the presentation engine with the work we'll actually perform.
		// If we provide a timeout, it will also block the CPU until an image is available, which can allow us to pace render frames in conjunction with presentation frames.
		// The image index returned is the index into the array of VkImages returned from vkGetSwapchainImagesKHR.  We use it to select our framebuffer this frame.
		VkResult result = vkAcquireNextImageKHR( barebonesRenderer.device, barebonesRenderer.swapchain, INFINITE, VK_NULL_HANDLE, VK_NULL_HANDLE, &imageIndex );
		// Command buffers must be "begun" to start recording commands.  Any "vkCmd" call on a command buffer will record the next command to be executed in serial.
		VkCommandBufferBeginInfo commandBufferBeginInfo = {};
		commandBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		// This flag is a hint that the command buffer will not be submitted multiple times without being re-recorded.  It can potentially cause some optimizations,
		// and generally, we don't submit command buffers multiple times because the contents tend to change frame to frame, at least by a little.
		commandBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		result = vkBeginCommandBuffer( barebonesRenderer.commandBuffer, &commandBufferBeginInfo );
		VkClearValue clearColor = {};
		clearColor.color.float32[ 0 ] = 1.0f;
		clearColor.color.float32[ 1 ] = 1.0f;
		clearColor.color.float32[ 2 ] = 1.0f;
		clearColor.color.float32[ 3 ] = 1.0f;
		// We begin a render pass to set the swapchain image view as a color attachment
		VkRenderPassBeginInfo renderPassBeginInfo = {};
		renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassBeginInfo.renderPass = barebonesRenderer.renderPass;
		renderPassBeginInfo.framebuffer = barebonesRenderer.framebuffers[ imageIndex ];
		// We have to provide a render area, so we just choose the full image (this is what we'll do in every case).
		renderPassBeginInfo.renderArea.offset.x = 0;
		renderPassBeginInfo.renderArea.offset.y = 0;
		renderPassBeginInfo.renderArea.extent = barebonesRenderer.swapchainExtent;
		renderPassBeginInfo.clearValueCount = 1;	// Provide the clear value to verify that rendering works as expected
		renderPassBeginInfo.pClearValues = &clearColor;
		// Subpass contents inline means that no rendering code for this render pass will be in a secondary command buffer (we won't use any)
		vkCmdBeginRenderPass( barebonesRenderer.commandBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE );
		// The actual meat of the draw is as simple as binding the pipeline (one call) to the graphics bind point and making the draw command.
		// Later, we'll add descriptor sets (for shader resources) to the mix, but binding those can still be just one extra call.
		vkCmdBindPipeline( barebonesRenderer.commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, barebonesRenderer.pipeline );
		// We use no vertex buffer because unbound drawing will just generate vertex indices 0, 1, and 2, to construct a triangle entirely in shader logic.
		vkCmdDraw( barebonesRenderer.commandBuffer, 3, 1, 0, 0 );
		vkCmdEndRenderPass( barebonesRenderer.commandBuffer );	// Any begun render pass must be ended
		result = vkEndCommandBuffer( barebonesRenderer.commandBuffer );	// And we stop recording to the command buffer by "ending" it
		// Next, we fill out a submit info.  Multiple command buffers can be submitted in a single info, and multiple infos can be submitted in a single call.
		// Command buffers are executed in order of submission, and semaphores can be used to signal or wait on different queues.
		// Normally, we'd have a semaphore signaled by the swapchain image acquire to wait here, and this would signal a separate semaphore to be waited on in the present call,
		// but we brute-force flushes to avoid these (semaphores allow cross-queue synchronization, and the presentation engine is not controlled by the app).  Technically,
		// it is still incorrect not to use semaphores to synchronize the presentation engine with our usage of swapchain images through acquisition and rendering, but
		// we'll just categorize that as "out of scope" for this sprint and handle it later.
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &barebonesRenderer.commandBuffer;
		result = vkQueueSubmit( barebonesRenderer.queue, 1, &submitInfo, VK_NULL_HANDLE );
		result = vkQueueWaitIdle( barebonesRenderer.queue );	// Force the work to be done before continuing (this is bad form for a render loop)
		VkPresentInfoKHR presentInfo = {};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &barebonesRenderer.swapchain;
		presentInfo.pImageIndices = &imageIndex;
		result = vkQueuePresentKHR( barebonesRenderer.queue, &presentInfo );	// And present the swapchain image to the screen
	}
}