#pragma once

#include "Platform/Keycodes.h"
#include <vector>

namespace Cosmos
{
	class Widget
	{
	public:

		// constructor
		Widget(const char* name = "Widget");

		// destructor
		virtual ~Widget() = default;

		// returns it's name
		inline const char* Name() { return mName; }

	public:

		// for user interface drawing
		virtual void OnUpdate() {}

		// for renderer drawing
		virtual void OnRenderDraw() {}

	public:

		// // called when the window is resized
		virtual void OnWindowResize() {}

		// called when a mouse position changes
		virtual void OnMouseMove(float x, float y) {}

		// called when a mouse scroll happens
		virtual void OnMouseScroll(float y) {}

		// called when a mouse button was pressed
		virtual void OnMousePress(Buttoncode button) {}

		// called when a mouse button was released
		virtual void OnMouseRelease(Buttoncode button) {}

		// called when a keyboard key is pressed
		virtual void OnKeyboardPress(Keycode key) {}

		// called when a keyboard key is released
		virtual void OnKeyboardRelease(Keycode key) {}

	private:

		const char* mName;
	};

	class WidgetStack
	{
	public:

		// constructor
		WidgetStack() = default;

		// destructor
		~WidgetStack() = default;

		// returns a reference to the elements vector
		inline std::vector<Widget*>& GetWidgets() { return mWidgets; }

	public:

		// pushes an ui to the top half of the stack
		void PushOver(Widget* ent);

		// pops an ui element from the top half of the stack
		void PopOver(Widget* ent);

		// pushes an ui to the bottom half of the stack
		void Push(Widget* ent);

		// pops an ui element from the bottom half of the stack
		void Pop(Widget* ent);

	public:

		// iterators
		std::vector<Widget*>::iterator begin() { return mWidgets.begin(); }
		std::vector<Widget*>::iterator end() { return mWidgets.end(); }
		std::vector<Widget*>::reverse_iterator rbegin() { return mWidgets.rbegin(); }
		std::vector<Widget*>::reverse_iterator rend() { return mWidgets.rend(); }
		std::vector<Widget*>::const_iterator begin() const { return mWidgets.begin(); }
		std::vector<Widget*>::const_iterator end()	const { return mWidgets.end(); }
		std::vector<Widget*>::const_reverse_iterator rbegin() const { return mWidgets.rbegin(); }
		std::vector<Widget*>::const_reverse_iterator rend() const { return mWidgets.rend(); }

	private:

		std::vector<Widget*> mWidgets;
		uint32_t mMiddlePos = 0;
	};
}