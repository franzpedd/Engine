#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Dockspace : public UIElement
	{
	public:

		// constructor
		Dockspace();

		// destructor
		virtual ~Dockspace();

	public:

		// updates the ui element
		virtual void OnUpdate() override;

		// window was recently resized
		virtual void OnResize() override;
	};
}