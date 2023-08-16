#pragma once

#include <vector>

namespace Cosmos
{
	class UIElement
	{
	public:

		// constructor
		UIElement() = default;

		// destructor
		virtual ~UIElement() = default;

	public:

		// updates the ui element
		virtual void OnUpdate() = 0;

		// window was recently resized
		virtual void OnResize() = 0;
	};

	// this class is a vector wrapper to manage multiple ui elements
	class UIElementStack
	{
	public:

		// constructor
		UIElementStack();

		// destructor
		~UIElementStack();

		// returns a reference to the elements vector
		std::vector<UIElement*>& Elements() { return mElements; }

	public:

		// pushes an ui to the top half of the stack
		void PushOver(UIElement* element);

		// pops an ui element from the top half of the stack
		void PopOver(UIElement* element);

		// pushes an ui to the bottom half of the stack
		void Push(UIElement* element);

		// pops an ui element from the bottom half of the stack
		void Pop(UIElement* element);

	public:

		// iterators
		std::vector<UIElement*>::iterator begin() { return mElements.begin(); }
		std::vector<UIElement*>::iterator end() { return mElements.end(); }
		std::vector<UIElement*>::reverse_iterator rbegin() { return mElements.rbegin(); }
		std::vector<UIElement*>::reverse_iterator rend() { return mElements.rend(); }
		std::vector<UIElement*>::const_iterator begin() const { return mElements.begin(); }
		std::vector<UIElement*>::const_iterator end()	const { return mElements.end(); }
		std::vector<UIElement*>::const_reverse_iterator rbegin() const { return mElements.rbegin(); }
		std::vector<UIElement*>::const_reverse_iterator rend() const { return mElements.rend(); }

	private:

		std::vector<UIElement*> mElements;
		uint32_t mMiddlePos = 0;
	};
}