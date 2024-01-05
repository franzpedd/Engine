#pragma once

#include <Engine.h>
#include "Project.h"

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
	class TextureBrowser;
	class Viewport;

	class Editor : public Application
	{
	public:

		// constructor
		Editor();

		// destructor
		virtual ~Editor() = default;

	private:

		// project related
		std::unique_ptr<Project> mProject;

		// ui
		Console* mConsole;
		Dockspace* mDockspace;
		Explorer* mExplorer;
		Viewport* mViewport;
		Mainmenu* mMainmenu;
		SceneHierarchy* mSceneHierarchy;
		TextureBrowser* mTextureBrowser;

		// entity
		Camera* mCamera;
		Grid* mGrid;
		Gizmo* mGizmo;
	};
}