#pragma once

#include "Entity/Renderable/Model.h"

namespace Cosmos
{
	struct ModelComponent
	{
		std::shared_ptr<Model> model;

		// constructor
		ModelComponent() = default;
	};
}