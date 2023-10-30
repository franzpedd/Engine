#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Dockspace : public Entity
	{
	public:

		// constructor
		Dockspace();

		// destructor
		virtual ~Dockspace() = default;

	public:

		// updates the ui element
		virtual void OnUIDraw() override;
	};
}