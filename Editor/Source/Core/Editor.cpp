#include "Editor.h"

#include "Renderer/Grid.h"

#include "UI/Dockspace.h"
#include "UI/Explorer.h"
#include "UI/Viewport.h"

namespace Cosmos
{
	Editor::Editor()
		: mCamera(mWindow)
	{
		mDockspace = new Dockspace();
		mExplorer = new Explorer(mRenderer);
		mViewport = new Viewport(mUI, mRenderer);
		mGrid = new Grid(mRenderer, mCamera, *mViewport);

		mUI->ElementStack().Push(mDockspace);
		mUI->ElementStack().Push(mViewport);
		mUI->ElementStack().Push(mExplorer);

		mScene->Entities().Push(mGrid);
	}

	Editor::~Editor()
	{

	}

	void Editor::OnUpdate(Timestep ts)
	{
		mCamera.OnUpdate(ts);
	}

	void Editor::OnMouseMove(float xPos, float yPos, float xOffset, float yOffset)
	{
		mCamera.OnMouseMove(xOffset, yOffset);
	}

	void Editor::OnMouseScroll(float yOffset)
	{
		mCamera.OnMouseScroll(yOffset);
	}

	void Editor::OnMousePress(Buttoncode button)
	{
		
	}

	void Editor::OnMouseRelease(Buttoncode button)
	{
		
	}

	void Editor::OnKeyboardPress(Keycode key)
	{
		mCamera.OnKeyboardPress(key);
	}

	void Editor::OnKeyboardRelease(Keycode key)
	{

	}
}