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
		mCamera = new Camera(mWindow);

		mDockspace = new Dockspace();
		mExplorer = new Explorer(mRenderer);
		mViewport = new Viewport(mUI, mRenderer, mCamera);
		mGrid = new Grid(mRenderer, mCamera);
		mGizmo = new Gizmo(mWindow, mRenderer, mCamera);
		mMainmenu = new Mainmenu(mCamera, mGrid);
		mSceneHierarchy = new SceneHierarchy(mScene, mGizmo);

		// widgets
		mUI->Widgets().Push(mDockspace);
		mUI->Widgets().Push(mViewport);
		mUI->Widgets().Push(mExplorer);
		mUI->Widgets().Push(mMainmenu);
		mUI->Widgets().Push(mSceneHierarchy);
		
		// objects
		mScene->Entities().Push(mCamera);
		mScene->Entities().Push(mGrid);
		mScene->Entities().Push(mGizmo);

		// testing Primitives
		mScene->Entities().Push(new Plane(mRenderer, *mCamera));
		mScene->Entities().Push(new Plane(mRenderer, *mCamera));
		mScene->Entities().Push(new Cube(mRenderer, *mCamera));

		// todos
		LOG_TO_TERMINAL(Logger::Severity::Warn, "TODO: Rework Window hovering to consider inside docking window width and height (EDITOR)");
		LOG_TO_TERMINAL(Logger::Severity::Warn, "TODO: Create Event Listener System for only calling those who are listening the events and not all Entities/widgets (ENGINE)");
	}
}