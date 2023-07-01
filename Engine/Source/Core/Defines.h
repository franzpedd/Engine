#pragma once

// --------------------------------------------------------------------------

#if defined(__clang__) || defined(__gcc__)
    #define STATIC_ASSERT _Static_assert
#else
    #define STATIC_ASSERT static_assert
#endif

// --------------------------------------------------------------------------

#if _MSC_VER
	#define debugBreak() __debugbreak()
#else
	#define debugBreak() __builtin_trap()
#endif

// --------------------------------------------------------------------------

typedef unsigned char u8;
typedef unsigned short u16;
typedef unsigned int u32;
typedef unsigned long long u64;
typedef signed char i8;
typedef signed short i16;
typedef signed int i32;
typedef signed long long i64;
typedef float f32;
typedef double f64;

STATIC_ASSERT(sizeof(u8) == 1, "Expected u8 to be 1 byte.");
STATIC_ASSERT(sizeof(u16) == 2, "Expected u16 to be 2 bytes.");
STATIC_ASSERT(sizeof(u32) == 4, "Expected u32 to be 4 bytes.");
STATIC_ASSERT(sizeof(u64) == 8, "Expected u64 to be 8 bytes.");
STATIC_ASSERT(sizeof(i8) == 1, "Expected i8 to be 1 byte.");
STATIC_ASSERT(sizeof(i16) == 2, "Expected i16 to be 2 bytes.");
STATIC_ASSERT(sizeof(i32) == 4, "Expected i32 to be 4 bytes.");
STATIC_ASSERT(sizeof(i64) == 8, "Expected i64 to be 8 bytes.");
STATIC_ASSERT(sizeof(f32) == 4, "Expected f32 to be 4 bytes.");
STATIC_ASSERT(sizeof(f64) == 8, "Expected f64 to be 8 bytes.");

// --------------------------------------------------------------------------

#if defined(_WIN64)
    #define PLATFORM_WINDOWS
#elif defined(__APPLE__)
    #include <TargetConditionals.h>
    #if defined(TARGET_OS_IPHONE)
        #define PLATFORM_IPHONE
    #elif defined(TARGET_OS_MAC)
        #define PLATFORM_MAC
    #endif
#elif
#elif defined(__ANDROID__)
    #define PLATFORM_ANDROID
#elif defined(__linux__) || defined(__gnu_linux__)
    #define PLATFORM_LINUX
    #error "Unsupported platform";
#endif

// --------------------------------------------------------------------------

#define RENDERER_VULKAN // this enables vulkan code to be implemented in certain files

// --------------------------------------------------------------------------

// --------------------------------------------------------------------------

#if defined(API_EXPORT)
	#if defined(_MSC_VER)
		#define COSMOS_API __declspec(dllexport)
	#else
		#define COSMOS_API __attribute__((visibility("default")))
	#endif
#elif defined(API_IMPORT)
    #if defined(_MSC_VER)
        #define COSMOS_API __declspec(dllimport)
    #else
        #define COSMOS_API
    #endif
#endif
