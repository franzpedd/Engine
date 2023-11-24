#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Dockspace : public Widget
	{
	public:

		// constructor
		Dockspace();

		// destructor
		virtual ~Dockspace() = default;

	public:

		// updates the ui element
		virtual void OnUpdate() override;
	};
}