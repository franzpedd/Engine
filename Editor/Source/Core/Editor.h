#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declarations
	class Dockspace;
	class Explorer;
	class Grid;
	class Mainmenu;
	class Viewport;

	class Editor : public Application
	{
	public:

		// constructor
		Editor();

		// destructor
		~Editor();

	public:

		// updates the editor logic
		void OnUpdate(float timestep) override;

	public:
		
		// called when a mouse was moved
		virtual void OnMouseMove(float xPos, float yPos, float xOffset, float yOffset) override;

		// called when a mouse was scrolled
		virtual void OnMouseScroll(float yOffset) override;

		// called when a mouse button was pressed
		virtual void OnMousePress(Buttoncode button) override;

		// called when a mouse button was released
		virtual void OnMouseRelease(Buttoncode button) override;

		// called when a keyboard key is pressed
		virtual void OnKeyboardPress(Keycode key) override;

		// called when a keyboard key is released
		virtual void OnKeyboardRelease(Keycode key) override;

	private:

		Dockspace* mDockspace;
		Explorer* mExplorer;
		Grid* mGrid;
		Viewport* mViewport;
		Mainmenu* mMainmenu;

		Camera mCamera;
	};
}