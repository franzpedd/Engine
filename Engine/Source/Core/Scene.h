#pragma once

#include "Entity/Entt.h"
#include "Platform/Keycodes.h"
#include "Util/UUID.h"
#include "Util/DataFile.h"
#include <memory>

namespace Cosmos
{
	// forward declarations
	class Entity;
	class EntityStack;
	class Window;
	class Renderer;
	class Camera;
	class GUI;

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
		inline std::unordered_map<std::string, Entity>& GetEntityMap() { return mEntityMap; }

		// returns a renderer reference
		inline std::shared_ptr<Camera>& GetCamera() { return mCamera; }

		// connects the scene to the renderer
		inline void ConnectRenderer(std::shared_ptr<Renderer> renderer) { mRenderer = renderer; }

		// connects the ui with the scene
		void ConnectUI(std::shared_ptr<GUI> gui);

		// returns a renderer reference
		inline std::shared_ptr<Renderer>& GetRenderer() { return mRenderer; }

	public:

		// creates a new entt entity
		void CreateEntity(const char* name = "Entity");

		// deleste an entity and free it's resources
		void DestroyEntity(Entity* entity);

		// finds an entity given it's id (wich is unique)
		Entity* FindEntityById(UUID id);

	public:

		// updates the scene objects
		void OnUpdate(float timestep);

		// draws the scene drawables
		void OnRenderDraw();

		// cleans the entities resources
		void Destroy();

		// erases all entities(that contain components) of the scene
		void CleanCurrentScene();

		// loads a new scene
		void Load(DataFile& sceneEntities);

		// serializes the scene and returns a structure with it serialized
		DataFile Serialize();

	public:

		// mouse was recently moved
		void OnMouseMove(float x, float y);

		// mouse was recently scrolled
		void OnMouseScroll(float y);

		// keyboard key was recrently pressed
		void OnKeyboardPress(Keycode key);

	private:

		std::shared_ptr<Window>& mWindow;
		std::unique_ptr<EntityStack> mEntities;
		entt::registry mRegistry;

		// must be connected, at the moment the scene is initialized before renderer
		std::shared_ptr<Renderer> mRenderer;

		// must be connected, to pass view and projection for entities
		std::shared_ptr<Camera> mCamera;

		// must be connected, to draw custom widgets that contain draw calls
		std::shared_ptr<GUI> mGUI;

		std::unordered_map<std::string, Entity> mEntityMap;
	};
}