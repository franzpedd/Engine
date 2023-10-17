#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declarations
	class Dockspace;
	class Explorer;
	class Grid;
	class Viewport;

	class Editor : public Application
	{
	public:

		// constructor
		Editor();

		// destructor
		~Editor();

	public:
		
		// called when a mouse move event was fired
		virtual void OnMouseMove(float xPos, float yPos, float xOffset, float yOffset) override;

		// called when a mouse scroll event was fired
		virtual void OnMouseScroll(float yOffset) override;

		// called when a keyboard key is pressed
		virtual void OnKeyboardPress(Keycode key) override;

	private:

		Dockspace* mDockspace;
		Explorer* mExplorer;
		Grid* mGrid;
		Viewport* mViewport;

	};
}