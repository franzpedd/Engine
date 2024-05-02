#pragma once

#include "Entity/Renderable/Model.h"
#include "Entity/Renderable/Primitive.h"

namespace Cosmos
{
	struct ModelComponent
	{
		Shared<Model> model;

		// constructor
		ModelComponent() = default;
	};

	struct QuadComponent
	{
		Shared<Quad> quad;

		// constructor
		QuadComponent() = default;
	};
}