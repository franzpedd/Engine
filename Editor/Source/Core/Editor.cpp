#include "Editor.h"

#include "Renderer/Gizmo.h"
#include "Renderer/Grid.h"

#include "UI/Console.h"
#include "UI/Dockspace.h"
#include "UI/Explorer.h"
#include "UI/Mainmenu.h"
#include "UI/ModelBrowser.h"
#include "UI/SceneHierarchy.h"
#include "UI/TextureBrowser.h"
#include "UI/Viewport.h"

namespace Cosmos
{
	Editor::Editor()
	{
		mProject = std::make_unique<Project>(mScene, "Project");

		mCamera = new Camera(mWindow, mScene);

		mConsole = new Console();
		mExplorer = new Explorer(mRenderer);
		mModelBrowser = new ModelBrowser(mRenderer);
		mTextureBrowser = new TextureBrowser(mRenderer);
		mViewport = new Viewport(mUI, mRenderer, mCamera, mTextureBrowser, mModelBrowser);
		mGrid = new Grid(mRenderer, mScene, mCamera);
		mGizmo = new Gizmo(mWindow, mRenderer, mScene, mCamera, mViewport);
		mMainmenu = new Mainmenu(mProject, mCamera, mGrid);
		mSceneHierarchy = new SceneHierarchy(mScene, mGizmo);
		mDockspace = new Dockspace();

		// widgets
		mUI->Widgets().Push(mDockspace);
		mUI->Widgets().Push(mConsole);
		mUI->Widgets().Push(mExplorer);
		mUI->Widgets().Push(mMainmenu);
		mUI->Widgets().Push(mModelBrowser);
		mUI->Widgets().Push(mSceneHierarchy);
		mUI->Widgets().Push(mTextureBrowser);
		mUI->Widgets().Push(mViewport); // viewport over everything
		
		// objects
		mScene->Entities()->Push(mCamera);
		mScene->Entities()->Push(mGrid);
		//mScene->Entities()->Push(mGizmo);

		// testing Primitives
		//mScene->Entities()->Push(new Plane(mScene, mRenderer, *mCamera));
		//mScene->Entities()->Push(new Cube(mScene, mRenderer, *mCamera));

		// todos
		LOG_TO_TERMINAL(Logger::Todo, "Implement Platform Safe C functions");
		LOG_TO_TERMINAL(Logger::Todo, "Fix Mainloop timestep and fps system");
		LOG_TO_TERMINAL(Logger::Todo, "Rework Window hovering to consider inside docking window width and height (EDITOR)");
		LOG_TO_TERMINAL(Logger::Todo, "Create Event Listener System for only calling those who are listening the events and not all Entities/widgets (ENGINE)");
		LOG_TO_TERMINAL(Logger::Todo, "Fix new layout for existing windows");
		LOG_TO_TERMINAL(Logger::Todo, "Move side menu to viewport overlay");
		LOG_TO_TERMINAL(Logger::Todo, "ECS: Implement Texture Component");
		LOG_TO_TERMINAL(Logger::Todo, "TextureBrowser: Add default texture on disk for initialization");
		LOG_TO_TERMINAL(Logger::Todo, "Model: Add support for GLB files after initial model support");
		LOG_TO_TERMINAL(Logger::Todo, "Only convert Model's texture if device supports RGB only");
		LOG_TO_TERMINAL(Logger::Todo, "Move all devices requirements to device creation (VKTexture.cpp)");
		LOG_TO_TERMINAL(Logger::Todo, "Modify CreateImageView() to support custom miplevels");
	}
}