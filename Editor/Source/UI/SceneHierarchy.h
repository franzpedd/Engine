#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declaration
	class Gizmo;

	class SceneHierarchy : public Widget
	{
	public:

		// constructor
		SceneHierarchy(Scene* scene);

		// destructor
		~SceneHierarchy();

		// for user interface drawing
		virtual void OnUpdate() override;

	public:

		// returns a pointer to the currently selected entity
		Entity* GetSelectedEntity();

		// unslects the selected entity (happens when a new scene loads)
		void UnselectEntity();

		// adds a menu option with the component name
		template<typename T>
		void DisplayAddComponentEntry(const char* name);

	private:

		// sub-menu with scene hierarchy
		void DisplaySceneHierarchy();

		// sub-menu with selected entity components on display
		void DisplaySelectedEntityComponents();

		// draws a existing entity node on hierarchy menu, redraw is used to check if entity map was altered and needs to be redrawn
		void DrawEntityNode(Entity* entity, bool* redraw);

		// draws all components an given entity has
		void DrawComponents(Entity* entity);

		// draws a single component and forwards the function
		template<typename T, typename F>
		static void DrawComponent(const char* name, Entity* entity, F func);

	private:

		Scene* mScene;
		Entity* mSelectedEntity = nullptr;
	};
}