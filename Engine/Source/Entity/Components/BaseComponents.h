#pragma once

#include "Util/Math.h"
#include "Util/UUID.h"

namespace Cosmos
{
	struct IDComponent
	{
		UUID id;

		// constructor
		IDComponent() = default;
	};

	struct NameComponent
	{
		std::string name;

		// constructor
		NameComponent() = default;
	};

	struct TransformComponent
	{
		glm::vec3 position = glm::vec3(1.0f);
		glm::vec3 rotation = glm::vec3(1.0f);
		glm::vec3 translation = glm::vec3(1.0f);

		// constructor
		TransformComponent() = default;
	};
}