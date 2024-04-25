#pragma once

#include "Defines.h"

#if defined(PLATFORM_WINDOWS)
#pragma warning( push )
#pragma warning( disable : 4244 26827 )
#endif
#include <entt.hpp>
#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif
