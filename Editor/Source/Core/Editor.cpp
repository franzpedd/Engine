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
	// defined on engine Platform/Main.h
	Application* CreateApplication()
	{
		return new Editor();
	}

	Editor::Editor()
	{
		mProject = std::make_unique<Project>("Project");

		mConsole = new Console();
		mExplorer = new Explorer(mRenderer);
		mTextureBrowser = new TextureBrowser(mRenderer);
		mSceneHierarchy = new SceneHierarchy(mRenderer, mCamera);
		mViewport = new Viewport(mRenderer, mSceneHierarchy, mTextureBrowser);
		mGrid = new Grid(mRenderer);
		mMainmenu = new Mainmenu(mProject, mGrid, mSceneHierarchy);
		mDockspace = new Dockspace();

		// widgets
		mUI->Widgets().Push(mDockspace);
		mUI->Widgets().Push(mConsole);
		mUI->Widgets().Push(mExplorer);
		mUI->Widgets().Push(mMainmenu);
		mUI->Widgets().Push(mSceneHierarchy);
		mUI->Widgets().Push(mTextureBrowser);
		mUI->Widgets().Push(mGrid);
		mUI->Widgets().Push(mViewport); // viewport over everything

		DisplayToDoList();
	}

	Editor::~Editor()
	{
		delete mDockspace;
		delete mMainmenu;
		delete mGrid;
		delete mViewport;
		delete mSceneHierarchy;
		delete mTextureBrowser;
		delete mExplorer;
		delete mConsole;
	}

	void Editor::DisplayToDoList()
	{
		LOG_TO_TERMINAL(Logger::Todo, "Application: Implement an Event System instead of manually forwarding input events to widgets");
		LOG_TO_TERMINAL(Logger::Todo, "Application: Fix order of construction all the way to renderer and resources");
		LOG_TO_TERMINAL(Logger::Todo, "Renderer: Re-arrange stuff");
		LOG_TO_TERMINAL(Logger::Todo, "Renderer: Make singleton");
		
		LOG_TO_TERMINAL(Logger::Todo, "Fix Project loading");
		LOG_TO_TERMINAL(Logger::Todo, "Project path is saved absolutely, witch will create an error when loading on another pc. Make them relative.");
		LOG_TO_TERMINAL(Logger::Todo, "Fix Mainloop timestep and fps system");
		LOG_TO_TERMINAL(Logger::Todo, "Rework Window hovering to consider inside docking window width and height (EDITOR)");
		LOG_TO_TERMINAL(Logger::Todo, "Create Event Listener System for only calling those who are listening the events and not all Entities/widgets (ENGINE)");
		LOG_TO_TERMINAL(Logger::Todo, "Fix new layout for existing windows");
		LOG_TO_TERMINAL(Logger::Todo, "Move all devices requirements to device creation (VKTexture.cpp)");
	}
}