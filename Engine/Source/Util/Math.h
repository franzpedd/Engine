#pragma once

#include "wrapper_glm.h"

namespace Cosmos::Math
{
	// decomposes a model matrix to translations, rotation and scale components
	bool Decompose(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
}