#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declarations
	class Dockspace;
	class Explorer;
	class Grid;
	class Viewport;

	class Editor : public Application
	{
	public:

		// constructor
		Editor();

		// destructor
		~Editor();

	private:

		Dockspace* mDockspace;
		Explorer* mExplorer;
		Grid* mGrid;
		Viewport* mViewport;

	};
}