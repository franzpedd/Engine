#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Explorer : public UIElement
	{
	public:

		// constructor
		Explorer();

		// destructor
		virtual ~Explorer();

	public:

		// updates the ui element
		virtual void OnUpdate() override;

		// window was recently resized
		virtual void OnResize() override;

	};
}