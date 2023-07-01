#pragma once

#include "Core/Defines.h"

#if defined RENDERER_VULKAN

#if defined (PLATFORM_WINDOWS)
    #define VK_USE_PLATFORM_WIN32_KHR 
#elif defined (PLATFORM_IPHONE)
    #define VK_USE_PLATFORM_IOS_MVK
#elif defined (PLATFORM_MAC)
    #define VK_USE_PLATFORM_MACOS_MVK
#elif defined (PLATFORM_ANDROID)
    #define VK_USE_PLATFORM_ANDROID_KHR
#elif defined (PLATFORM_LINUX)
    #define VK_USE_PLATFORM_WAYLAND_KHR // using wayland for linux windowing system
#endif

#include <vulkan/vulkan.h>

namespace Cosmos
{
	// returns the renderer api required extensions used by the platform
	const char* GetRendererAPIRequiredExtensions();

	// creates a window surface for the platform
	VkResult CreateWindowSurface(VkInstance instance, const VkAllocationCallbacks* allocator, VkSurfaceKHR* surface);
}

#endif