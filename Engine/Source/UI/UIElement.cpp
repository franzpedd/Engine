#include "UIElement.h"

namespace Cosmos
{
	UIElementStack::UIElementStack()
	{
	}

	UIElementStack::~UIElementStack()
	{
		for (UIElement* element : mElements)
		{
			delete element;
		}
	}

	void UIElementStack::PushOver(UIElement* element)
	{
		mElements.emplace_back(element);
	}

	void UIElementStack::PopOver(UIElement* element)
	{
		auto it = std::find(mElements.begin() + mMiddlePos, mElements.end(), element);
		if (it != mElements.end())
		{
			mElements.erase(it);
		}
	}

	void UIElementStack::Push(UIElement* element)
	{
		mElements.emplace(mElements.begin() + mMiddlePos, element);
		mMiddlePos++;
	}

	void UIElementStack::Pop(UIElement* element)
	{
		auto it = std::find(mElements.begin(), mElements.begin() + mMiddlePos, element);
		if (it != mElements.begin() + mMiddlePos)
		{
			mElements.erase(it);
			mMiddlePos--;
		}
	}
}