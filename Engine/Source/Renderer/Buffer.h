#pragma once

#include "Util/Math.h"

namespace Cosmos
{
	// model view projection matrix for any objects
	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};
}