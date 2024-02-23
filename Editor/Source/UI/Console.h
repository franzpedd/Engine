#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Console : public Widget
	{
	public:

		// constructor
		Console();

		// destructor
		virtual ~Console() = default;

	public:

		// for user interface drawing
		virtual void OnUpdate() override;

	private:

		bool mOpened = false;
	};
}