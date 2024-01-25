#pragma once

#include "Util/Math.h"

namespace Cosmos::ModelHelper
{
	struct Vertex
	{
		glm::vec3 position = {};
		glm::vec3 normal = {};
		glm::vec2 uv0 = {};
		glm::vec2 uv1 = {};
		glm::vec4 joint = {};
		glm::vec4 weight = {};
		glm::vec4 color = {};
	};
}