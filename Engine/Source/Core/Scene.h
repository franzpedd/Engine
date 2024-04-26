#pragma once

#include "Camera.h"
#include "Util/UUID.h"
#include "Util/DataFile.h"

#include "wrapper_entt.h"
#include <memory>

namespace Cosmos
{
	// forward declarations
	class Entity;
	class Window;
	class Renderer;
	class GUI;

	class Scene : public std::enable_shared_from_this<Scene>
	{
	public:

		// constructor
		Scene(Shared<Renderer> renderer);

		// destructor
		~Scene();

	public:

		// returns a smart reference to the scene object
		std::shared_ptr<Scene> GetSmartRef() { return shared_from_this(); }

		// returns a reference to the registry
		inline entt::registry& GetRegistry() { return mRegistry; }

		// returns a reference to the entity unordered map
		inline std::unordered_map<std::string, Entity>& GetEntityMap() { return mEntityMap; }

		// returns a renderer reference
		inline std::shared_ptr<Renderer>& GetRenderer() { return mRenderer; }

	public:

		// creates a new entt entity
		Entity* CreateEntity(const char* name = "Entity");

		// deleste an entity and free it's resources
		void DestroyEntity(Entity* entity);

		// finds an entity given it's id (wich is unique)
		Entity* FindEntityById(UUID id);

		// finds an entity given it's handle id
		Entity* FindEntityByHandle(entt::entity handle);

	public:

		// updates the scene objects
		void OnUpdate(float timestep);

		// draws the scene drawables
		void OnRender();

		// event handling
		void OnEvent(Shared<Event> event);

		// loads a new scene
		void Deserialize(DataFile& data);

		// serializes the scene and returns a structure with it serialized
		DataFile Serialize();

	private:

		Shared<Renderer> mRenderer;
		entt::registry mRegistry;

		// holds the scene entities
		std::unordered_map<std::string, Entity> mEntityMap = {};
	};
}