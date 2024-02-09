#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26495 26451 )
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Cosmos::Math
{
	// decomposes a model matrix to translations, rotation and scale components
	bool Decompose(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}