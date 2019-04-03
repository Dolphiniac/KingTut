#define VK_USE_PLATFORM_WIN32_KHR
#include "Renderer.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

static HWND hwnd;

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
	hwnd = CreateWindowExA( 0, className, "My grownup scene", windowStyle, rect.left, rect.top, width, height, NULL, NULL, hInstance, NULL );

	VkWin32SurfaceCreateInfoKHR surfaceCreateInfo = {};
	surfaceCreateInfo.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
	surfaceCreateInfo.hinstance = hInstance;
	surfaceCreateInfo.hwnd = hwnd;
	VkResult result = vkCreateWin32SurfaceKHR( renderObjects.instance, &surfaceCreateInfo, NULL, &renderObjects.surface );
}

const char * GetPlatformSurfaceExtensionName() {
	return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
}

void PumpMessages() {
	MSG msg;
	while ( PeekMessageA( &msg, hwnd, 0, 0, PM_REMOVE ) ) {
		TranslateMessage( &msg );
		DispatchMessageA( &msg );
	}
}