#pragma once

#include "Defines.h"

#if defined(PLATFORM_WINDOWS)
#pragma warning(push)
#pragma warning(disable : 26495 26451)
#endif

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtc/epsilon.hpp>

#if defined(PLATFORM_WINDOWS)
#pragma warning(pop)
#endif