#pragma once

#include "Entity/Renderable/Model.h"
#include "Entity/Renderable/Primitive.h"
#include "Entity/Renderable/Skybox.h"

namespace Cosmos
{
	struct ModelComponent
	{
		Shared<Model> model;

		// constructor
		ModelComponent() = default;
	};

	struct SkyboxComponent
	{
		Shared<Skybox> skybox;

		// constructor
		SkyboxComponent() = default;
	};

	struct QuadComponent
	{
		Shared<Quad> quad;

		// constructor
		QuadComponent() = default;
	};
}