#include "Editor.h"

#include "UI/Dockspace.h"
#include "UI/Explorer.h"
#include "UI/Viewport.h"

namespace Cosmos
{
	Editor::Editor()
	{
		mUI->ElementStack().Push(new Dockspace{});
		mUI->ElementStack().Push(new Viewport{ mUI, mRenderer });
		mUI->ElementStack().Push(new Explorer{ mRenderer });
	}

	Editor::~Editor()
	{

	}
}