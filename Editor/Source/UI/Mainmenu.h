#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declarations
	class Grid;

	class Mainmenu : public UIElement
	{
	public:

		// constructor
		Mainmenu(Camera& camera, Grid* grid);

		// destructor
		virtual ~Mainmenu();

	public:

		// updates the ui element
		virtual void OnUpdate() override;

		// window was recently resized
		virtual void OnResize() override;

	private:

		// drawing and logic of main menu
		void DisplayMainMenu();

	private:

		Camera& mCamera;
		Grid* mGrid;
		
		bool mCheckboxGrid = true;
	};
}