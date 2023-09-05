#pragma once

#include <memory>

namespace Cosmos
{
	// forward declarations
	class Renderer;

	// controlls entity update cycle
	// controlls subsystem ? like lights/foliage/climate/terrain

	class Scene
	{
	public:
		
		// returns a smart pointer to a new scene
		static std::shared_ptr<Scene> Create(std::shared_ptr<Renderer>& renderer);
	
		// constructor
		Scene(std::shared_ptr<Renderer>& renderer);

		// destructor
		~Scene();

	public:



	private:

		std::shared_ptr<Renderer>& mRenderer;
	};
}