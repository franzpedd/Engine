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
		mGrid = new Grid(mRenderer, mScene->GetCamera(), *mViewport);

		mUI->ElementStack().Push(mDockspace);
		mUI->ElementStack().Push(mViewport);
		mUI->ElementStack().Push(mExplorer);

		mScene->Entities().Push(mGrid);
	}

	Editor::~Editor()
	{

	}

	void Editor::OnMouseMove(float xPos, float yPos, float xOffset, float yOffset)
	{
		mScene->GetCamera().OnMouseMove(xOffset, yOffset);
	}

	void Editor::OnMouseScroll(float yOffset)
	{
		mScene->GetCamera().OnMouseScroll(yOffset);
	}

	void Editor::OnKeyboardPress(Keycode key)
	{
		mScene->GetCamera().OnKeyboardPress(key);
	}
}