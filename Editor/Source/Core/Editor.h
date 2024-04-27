#pragma once

#include <Engine.h>
#include <Platform/Main.h>

#include "Project.h"

namespace Cosmos
{
	// forward declarations
	class Console;
	class Dockspace;
	class Explorer;
	class Grid;
	class ImDemo;
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
		virtual ~Editor();


	private:

		// place where I leave all todos
		void DisplayToDoList();

	private:

		// project related
		std::unique_ptr<Project> mProject;

		// ui
		Console* mConsole = nullptr;
		Dockspace* mDockspace = nullptr;
		Explorer* mExplorer= nullptr;
		Viewport* mViewport= nullptr;
		Mainmenu* mMainmenu= nullptr;
		SceneHierarchy* mSceneHierarchy= nullptr;
		TextureBrowser* mTextureBrowser= nullptr;
		Grid* mGrid = nullptr;
		ImDemo* mImDemo = nullptr;
	};
}