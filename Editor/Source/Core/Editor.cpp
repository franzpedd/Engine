#include "Editor.h"

#include "Renderer/Gizmo.h"
#include "Renderer/Grid.h"

#include "UI/Dockspace.h"
#include "UI/Explorer.h"
#include "UI/Mainmenu.h"
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

		// components
		mScene->Entities().Push(mCamera);

		// widgets
		mScene->Entities().Push(mDockspace);
		mScene->Entities().Push(mViewport);
		mScene->Entities().Push(mExplorer);
		mScene->Entities().Push(mMainmenu);
		
		// objects
		mScene->Entities().Push(mGrid);
		mScene->Entities().Push(mGizmo);

		// testing Primitives
		mScene->Entities().Push(new Plane(mRenderer, *mCamera));
		mScene->Entities().Push(new Plane(mRenderer, *mCamera));
		mScene->Entities().Push(new Cube(mRenderer, *mCamera));

		LOG_TO_TERMINAL(Logger::Severity::Warn, "TODO: Rework Window hovering to consider inside docking window width and height");
	}
}