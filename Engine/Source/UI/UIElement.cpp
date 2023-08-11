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
			element->OnDeletion();
			delete element;
		}
	}

	void UIElementStack::PushOver(UIElement* element)
	{
		element->OnCreation();
		mElements.emplace_back(element);
	}

	void UIElementStack::PopOver(UIElement* element)
	{
		auto it = std::find(mElements.begin() + mMiddlePos, mElements.end(), element);
		if (it != mElements.end())
		{
			element->OnDeletion();
			mElements.erase(it);
		}
	}

	void UIElementStack::Push(UIElement* element)
	{
		element->OnCreation();
		mElements.emplace(mElements.begin() + mMiddlePos, element);
		mMiddlePos++;
	}

	void UIElementStack::Pop(UIElement* element)
	{
		auto it = std::find(mElements.begin(), mElements.begin() + mMiddlePos, element);
		if (it != mElements.begin() + mMiddlePos)
		{
			element->OnDeletion();
			mElements.erase(it);
			mMiddlePos--;
		}
	}
}