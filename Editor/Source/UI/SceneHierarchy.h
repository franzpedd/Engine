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
		SceneHierarchy(Scene* scene, Gizmo* gizmo);

		// destructor
		~SceneHierarchy();

		// for user interface drawing
		virtual void OnUpdateUI() override;

	public:

		// draws a single component, templated to forward function at the end
		template<typename T, typename F>
		void DrawSingleComponent(const char* name, Entity* entity, F function);

		// adds a menu option with the component name
		template<typename T>
		void DisplayAddComponentEntry(const char* name);

	private:

		// sub-menu with scene hierarchy
		void DisplaySceneHierarchy();

		// scene hierarchy edit menu-bar
		void DisplayEditMenubar();

		// sub-menu with selected entity components on display
		void DisplaySelectedEntityComponents();

		// draws a existing entity node on hierarchy menu
		void DrawEntityNode(Entity entity);

		// draws all components an given entity has
		void DrawComponents(Entity entity);

	private:

		Scene* mScene;
		Gizmo* mGizmo;

		Entity mSelectedEntity = {};
	};
}