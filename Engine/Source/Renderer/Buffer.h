#pragma once

#include "Util/Math.h"

namespace Cosmos
{
	// camera buffer
	struct UniformBufferObject
	{
		alignas(16) glm::mat4 model;
		alignas(16) glm::mat4 view;
		alignas(16) glm::mat4 proj;
	};

	// light buffer
	struct LightBufferObject
	{
		alignas(16) glm::vec4 light = { 1.0f, 1.0f, 1.0f, 1.0f }; // w is intensity
		alignas(16) glm::vec4 ambient = { 1.0f, 1.0f, 1.0f, 0.02f }; // w is intensity
		alignas(16) glm::vec3 position = { 0.0f, 0.0f, 2.5f };
	};
}