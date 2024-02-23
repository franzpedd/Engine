#pragma once

#include "Entity/Entt.h"
#include "Camera.h"
#include "Platform/Keycodes.h"
#include "Util/UUID.h"
#include "Util/DataFile.h"
#include <memory>

namespace Cosmos
{
	// forward declarations
	class Entity;
	class Window;
	class Renderer;
	class GUI;

	class Scene
	{
	public:

		// constructor
		Scene(std::shared_ptr<Window>& window);

		// destructor
		~Scene() = default;

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
		void OnRenderDraw();

		// cleans the entities resources
		void Destroy();

		// erases all entities(that contain components) of the scene
		void CleanCurrentScene();

		// loads a new scene
		void Deserialize(DataFile& data);

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
		entt::registry mRegistry;

		// must be connected, at the moment the scene is initialized before renderer
		std::shared_ptr<Renderer> mRenderer;

		// must be connected, to pass view and projection for entities
		std::shared_ptr<Camera> mCamera;

		// must be connected, to draw custom widgets that contain draw calls
		std::shared_ptr<GUI> mGUI;

		// holds the scene entities
		std::unordered_map<std::string, Entity> mEntityMap = {};
	};
}