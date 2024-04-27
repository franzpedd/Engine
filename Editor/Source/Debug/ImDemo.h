#pragma once

#include <Engine.h>

namespace Cosmos
{
	class ImDemo : public Widget
	{
	public:

		// constructor
		ImDemo() = default;

		// destructor
		virtual ~ImDemo() = default;

	public:

		// for user interface drawing
		virtual void OnUpdate() override;
	};
}