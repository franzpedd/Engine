#pragma once

#include "Defines.h"

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26819)
#endif

#if defined(PLATFORM_LINUX) && defined(INCLUDE_VULKAN_RENDERER)
#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>
#elif defined(PLATFORM_WINDOWS) && defined(INCLUDE_VULKAN_RENDERER)
#include <SDL.h>
#include <SDL_vulkan.h>
#endif

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif