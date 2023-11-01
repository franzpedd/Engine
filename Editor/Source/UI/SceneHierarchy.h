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
		SceneHierarchy(std::shared_ptr<Scene>& scene, Gizmo* gizmo);

		// destructor
		~SceneHierarchy();

		// for user interface drawing
		virtual void OnUpdateUI() override;

	private:

		std::shared_ptr<Scene>& mScene;
		Gizmo* mGizmo;
	};
}