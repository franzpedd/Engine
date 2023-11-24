#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declarations
	class Console;
	class Dockspace;
	class Explorer;
	class Grid;
	class Gizmo;
	class Mainmenu;
	class SceneHierarchy;
	class Viewport;

	class Editor : public Application
	{
	public:

		// constructor
		Editor();

		// destructor
		virtual ~Editor() = default;

	private:

		// ui
		Console* mConsole;
		Dockspace* mDockspace;
		Explorer* mExplorer;
		Viewport* mViewport;
		Mainmenu* mMainmenu;
		SceneHierarchy* mSceneHierarchy;

		// entity
		Grid* mGrid;
		Gizmo* mGizmo;

		Camera* mCamera;
	};
}