#pragma once

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26495 6255 6001)
#endif

#include <imguizmo.h>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif