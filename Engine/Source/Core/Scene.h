#pragma once

#include "Entity/Entt.h"
#include "Util/UUID.h"
#include "Util/Serializer.h"
#include <memory>

namespace Cosmos
{
	// forward declarations
	class Entity;
	class EntityStack;
	class Window;
	class Renderer;

	class Scene
	{
	public:

		// constructor
		Scene(std::shared_ptr<Window>& window);

		// destructor
		~Scene() = default;

		// returns a reference to the entities
		inline std::unique_ptr<EntityStack>& Entities() { return mEntities; }

		// returns a reference to the registry
		inline entt::registry& Registry() { return mRegistry; }

		// returns a reference to the entity unordered map
		inline std::unordered_map<UUID, entt::entity, UUID::Hash> GetEntityMap() { return mEntityMap; }

		// connects the scene to the renderer
		inline void ConnectRenderer(std::shared_ptr<Renderer> renderer) { mRenderer = renderer; }

		// returns a rendere's reference
		inline std::shared_ptr<Renderer>& GetRenderer() { return mRenderer; }

	public:

		// creates a new entt entity
		void CreateEntity(const char* name = "Entity");

		// deleste an entity and free it's resources
		void DestroyEntity(Entity entity);

		// finds an entity given it's name (wich may not be unique)
		Entity FindEntityByName(const char* name);

		// finds an entity given it's id (wich is unique)
		Entity FindEntityById(UUID id);

	public:

		// updates the scene objects
		void OnUpdate(float timestep);

		// draws the scene drawables
		void OnRenderDraw();

		// cleans the entities resources
		void Destroy();

		// loads a new scene
		void Load(Serializer& sceneEntities);

		// serializes the scene and returns a structure with it serialized
		Serializer Serialize();

	private:

		std::shared_ptr<Window>& mWindow;
		std::unique_ptr<EntityStack> mEntities;
		entt::registry mRegistry;

		// must be connected, at the moment the scene is initialized before renderer
		std::shared_ptr<Renderer> mRenderer;
		
		// stores all scene ents (will replace entitystack)
		std::unordered_map<UUID, entt::entity, UUID::Hash> mEntityMap;
	};
}