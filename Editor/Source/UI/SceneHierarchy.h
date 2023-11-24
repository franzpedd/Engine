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
		virtual void OnUpdate() override;

	public:

		// adds a menu option with the component name
		template<typename T>
		void DisplayAddComponentEntry(const char* name);

	private:

		// sub-menu with scene hierarchy
		void DisplaySceneHierarchy();

		// sub-menu with selected entity components on display
		void DisplaySelectedEntityComponents();

		// draws a existing entity node on hierarchy menu
		void DrawEntityNode(Entity entity);

		// draws all components an given entity has
		void DrawComponents(Entity entity);

		// draws a single component and forwards the function
		template<typename T, typename F>
		static void DrawComponent(const char* name, Entity& entity, F func);

	private:

		Scene* mScene;
		Gizmo* mGizmo;

		Entity mSelectedEntity = {};
	};
}