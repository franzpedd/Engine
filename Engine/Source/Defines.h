#pragma once

//// engine defines

// vulkan renderer
#define INCLUDE_VULKAN_RENDERER

// how many frames are simultaniously rendered on gpu
#define RENDERER_MAX_FRAMES_IN_FLIGHT 2

// how many chars in total an entity may have to represent it's name
#define ENTITY_NAME_MAX_CHARS 128

// how many chars a file may have in total (starting at 'Data')
#define MAX_SEARCH_PATH_SIZE 256

// how many threads the sound resources have
#define RESOURCES_THREAD_SOUND_COUNT 4


//// platform detection
// windows platform
#if defined(_WIN64)
	#define PLATFORM_WINDOWS

// linux platform
#elif defined(__linux__) || defined(__gnu_linux__)
	#define PLATFORM_LINUX
	#if defined(__ANDROID__)
		#define PLATFORM_ANDROID
	#endif

// apple platform
#elif __APPLE__
	#define PLATFORM_APPLE
	#include <TargetConditionals.h>
	#if TARGET_IPHONE_SIMULATOR
		#define PLATFORM_IOS
		#define PLATFORM_IOS_SIMULATOR
	#elif TARGET_OS_IPHONE
		#define PLATFORM_IOS
	#else
		#error "Unknown Apple platform"
	#endif
#elif

// unix platform
#elif defined(__unix__)
	#define PLATFORM_UNIX

// posix platform
#elif defined(_POSIX_VERSION)
	#define PLATFORM_POSIX

// unknown platform
#else
	#error "Unknown platform!"
#endif