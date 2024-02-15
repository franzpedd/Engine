#include "Editor.h"

#include "Renderer/Grid.h"

#include "UI/Console.h"
#include "UI/Dockspace.h"
#include "UI/Explorer.h"
#include "UI/Mainmenu.h"
#include "UI/SceneHierarchy.h"
#include "UI/TextureBrowser.h"
#include "UI/Viewport.h"

namespace Cosmos
{
	Editor::Editor()
	{
		mProject = std::make_unique<Project>(mScene, "Project");

		mConsole = new Console();
		mExplorer = new Explorer(mRenderer);
		mTextureBrowser = new TextureBrowser(mRenderer);
		mSceneHierarchy = new SceneHierarchy(mScene);
		mViewport = new Viewport(mUI, mRenderer, mScene->GetCamera(), mSceneHierarchy, mTextureBrowser);
		mGrid = new Grid(mRenderer, mScene);
		mMainmenu = new Mainmenu(mProject, mScene->GetCamera(), mGrid);
		mDockspace = new Dockspace();

		// widgets
		mUI->Widgets().Push(mDockspace);
		mUI->Widgets().Push(mConsole);
		mUI->Widgets().Push(mExplorer);
		mUI->Widgets().Push(mMainmenu);
		mUI->Widgets().Push(mSceneHierarchy);
		mUI->Widgets().Push(mTextureBrowser);
		mUI->Widgets().Push(mGrid); // very special indeed
		mUI->Widgets().Push(mViewport); // viewport over everything

		// todos
		LOG_TO_TERMINAL(Logger::Todo, "Fix Mainloop timestep and fps system");
		LOG_TO_TERMINAL(Logger::Todo, "Rework Window hovering to consider inside docking window width and height (EDITOR)");
		LOG_TO_TERMINAL(Logger::Todo, "Create Event Listener System for only calling those who are listening the events and not all Entities/widgets (ENGINE)");
		LOG_TO_TERMINAL(Logger::Todo, "Fix new layout for existing windows");
		LOG_TO_TERMINAL(Logger::Todo, "Move side menu to viewport overlay");
		LOG_TO_TERMINAL(Logger::Todo, "TextureBrowser: Add default texture on disk for initialization");
		LOG_TO_TERMINAL(Logger::Todo, "Move all devices requirements to device creation (VKTexture.cpp)");
	}
}