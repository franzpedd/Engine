#include "Widget.h"

namespace Cosmos
{
	Widget::Widget(const char* name)
		: mName(name)
	{
	}

	WidgetStack::~WidgetStack()
	{
		for (Widget* element : mWidgets)
		{
			delete element;
		}
	}

	void WidgetStack::PushOver(Widget* ent)
	{
		mWidgets.emplace_back(ent);
	}

	void WidgetStack::PopOver(Widget* ent)
	{
		auto it = std::find(mWidgets.begin() + mMiddlePos, mWidgets.end(), ent);
		if (it != mWidgets.end())
		{
			mWidgets.erase(it);
		}
	}

	void WidgetStack::Push(Widget* ent)
	{
		mWidgets.emplace(mWidgets.begin() + mMiddlePos, ent);
		mMiddlePos++;
	}

	void WidgetStack::Pop(Widget* ent)
	{
		auto it = std::find(mWidgets.begin(), mWidgets.begin() + mMiddlePos, ent);
		if (it != mWidgets.begin() + mMiddlePos)
		{
			mWidgets.erase(it);
			mMiddlePos--;
		}
	}
}