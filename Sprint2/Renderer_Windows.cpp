#define VK_USE_PLATFORM_WIN32_KHR	// Activate Windows-specific parts of Vulkan interface
#include "Renderer.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// Create Win32 surface and store into platform-independent VkSurfaceKHR
void CreateSurface() {
	HINSTANCE hInstance = GetModuleHandle( NULL );
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
	HWND window = CreateWindowExA( 0, className, "My grownup scene", windowStyle, rect.left, rect.top, width, height, NULL, NULL, hInstance, NULL );

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = hInstance;
	surfaceCreateInfo.hwnd = window;
	VkResult result = vkCreateWin32SurfaceKHR( renderObjects.instance, &surfaceCreateInfo, NULL, &renderObjects.surface );
}

// Return the string that activates the Win32 surface extension
const char * GetPlatformSurfaceExtensionName() {
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}