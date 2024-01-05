#pragma once

#include "Renderer/Texture.h"

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

	struct SpriteComponent
	{
		std::shared_ptr<Texture2D> sprite;

		// constructor
		SpriteComponent() = default;
	};
}