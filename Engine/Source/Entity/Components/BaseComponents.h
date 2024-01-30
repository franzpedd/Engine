#pragma once

#include "Entity/Specials/Model.h"
#include "Util/Math.h"
#include "Util/UUID.h"
#include "Util/Serializer.h"

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
		glm::vec3 translation = glm::vec3(0.0f);
		glm::vec3 rotation = glm::vec3(0.0f);
		glm::vec3 scale = glm::vec3(1.0f);

		// constructor
		TransformComponent() = default;
	};

	struct ModelComponent
	{
		Model* model;

		// constructor
		ModelComponent() = default;
	};

	struct SkyboxComponent
	{
		std::shared_ptr<void*> skybox;

		// constructor
		SkyboxComponent() = default;
	};
}