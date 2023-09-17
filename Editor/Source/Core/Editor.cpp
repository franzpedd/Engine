#include "Editor.h"

#include "Renderer/Grid.h"

#include "UI/Dockspace.h"
#include "UI/Explorer.h"
#include "UI/Viewport.h"

namespace Cosmos
{
	Editor::Editor()
	{
		mDockspace = new Dockspace();
		mExplorer = new Explorer(mRenderer);
		mViewport = new Viewport(mUI, mRenderer);
		mGrid = new Grid(mRenderer, *mViewport);

		mUI->ElementStack().Push(mDockspace);
		mUI->ElementStack().Push(mViewport);
		mUI->ElementStack().Push(mExplorer);

		mScene->Entities().Push(mGrid);
	}

	Editor::~Editor()
	{

	}
}