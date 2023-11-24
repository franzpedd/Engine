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
		~Console();

	public:

		// for user interface drawing
		virtual void OnUpdate() override;

		// freeing resources
		virtual void OnDestroy() override;

	private:

		bool mOpened = false;
	};
}