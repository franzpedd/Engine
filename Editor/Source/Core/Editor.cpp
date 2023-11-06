#include "Editor.h"

#include "Renderer/Gizmo.h"
#include "Renderer/Grid.h"

#include "UI/Dockspace.h"
#include "UI/Explorer.h"
#include "UI/Mainmenu.h"
#include "UI/SceneHierarchy.h"
#include "UI/Viewport.h"

namespace Cosmos
{
	Editor::Editor()
	{
		mCamera = new Camera(mWindow, mScene);

		mDockspace = new Dockspace();
		mExplorer = new Explorer(mRenderer);
		mViewport = new Viewport(mUI, mRenderer, mCamera);
		mGrid = new Grid(mRenderer, mScene, mCamera);
		mGizmo = new Gizmo(mWindow, mRenderer, mScene, mCamera, mViewport);
		mMainmenu = new Mainmenu(mCamera, mGrid);
		mSceneHierarchy = new SceneHierarchy(mScene, mGizmo);

		// widgets
		mUI->Widgets().Push(mDockspace);
		mUI->Widgets().Push(mViewport);
		mUI->Widgets().Push(mExplorer);
		mUI->Widgets().Push(mMainmenu);
		mUI->Widgets().Push(mSceneHierarchy);
		
		// objects
		mScene->Entities()->Push(mCamera);
		mScene->Entities()->Push(mGrid);
		mScene->Entities()->Push(mGizmo);

		// testing Primitives
		//mScene->Entities()->Push(new Plane(mScene, mRenderer, *mCamera));
		//mScene->Entities()->Push(new Cube(mScene, mRenderer, *mCamera));

		// todos
		LOG_TO_TERMINAL(Logger::Severity::Warn, "TODO: Fix Mainloop timestep");
		LOG_TO_TERMINAL(Logger::Severity::Warn, "TODO: Move from EntityStack to (ECS)");
		LOG_TO_TERMINAL(Logger::Severity::Warn, "TODO: Rework Window hovering to consider inside docking window width and height (EDITOR)");
		LOG_TO_TERMINAL(Logger::Severity::Warn, "TODO: Create Event Listener System for only calling those who are listening the events and not all Entities/widgets (ENGINE)");
	}
}