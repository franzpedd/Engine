#pragma once

#include "Camera.h"
#include "Event/Event.h"
#include "Renderer/Renderer.h"
#include "Util/DataFile.h"
#include "Util/Memory.h"
#include "Util/UUID.h"

#include "wrapper_entt.h"

namespace Cosmos
{
	// forward declarations
	class Entity;

	class Scene
	{
	public:

		// constructor
		Scene(Shared<Renderer> renderer, Shared<Camera> camera);

		// destructor
		~Scene();

	public:

		// returns a reference to the registry
		inline entt::registry& GetRegistryRef() { return mRegistry; }

		// returns a reference to the entity unordered map
		inline std::unordered_map<std::string, Entity>& GetEntityMapRef() { return mEntityMap; }

	public:

		// updates the scene objects
		void OnUpdate(float timestep);

		// draws the scene drawables
		void OnRender();

		// event handling
		void OnEvent(Shared<Event> event);

		// clears the scene
		void Clear();

	public:

		// creates a new entt entity
		Entity* CreateEntity(const char* name = "Entity");

		// duplicates an entity and insert's it on the entitymap
		void DuplicateEntity(Entity* entity);

		// deleste an entity and free it's resources
		void DestroyEntity(Entity* entity, bool eraseFromEntitymap = true);

		// finds an entity given it's id (wich is unique)
		Entity* FindEntityById(UUID id);

		// finds an entity given it's handle id
		Entity* FindEntityByHandle(entt::entity handle);

	public:

		// loads a new scene
		void Deserialize(DataFile& data);

		// serializes the scene and returns a structure with it serialized
		DataFile Serialize();

	private:

		Shared<Renderer> mRenderer;
		Shared<Camera> mCamera;
		entt::registry mRegistry;
		std::unordered_map<std::string, Entity> mEntityMap = {};
	};
}