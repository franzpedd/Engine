#pragma once

#include "Entity/Entity.h"
#include <memory>

namespace Cosmos
{
	// forward declarations

	class Scene
	{
	public:

		// returns a smart pointer to a new scene
		static std::shared_ptr<Scene> Create();

		// constructor
		Scene();

		// destructor
		~Scene();

		// returns a reference to the entities
		inline EntityStack& Entities() { return mEntities; }

	public:

		// updates the scene objects
		void OnUpdate();

		// draws the scene drawables
		void OnDraw();

		// cleans the entities resources
		void Destroy();

	private:

		EntityStack mEntities;
	};
}