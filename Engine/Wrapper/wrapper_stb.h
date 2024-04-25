#pragma once

#include "Defines.h"

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26827)
#endif

#include <stb_image.h>
#include <stb_image_write.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif