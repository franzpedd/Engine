#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declarations
	class Grid;

	class Mainmenu : public Entity
	{
	public:

		// constructor
		Mainmenu(Camera* camera, Grid* grid);

		// destructor
		virtual ~Mainmenu() = default;

	public:

		// updates the ui element
		virtual void OnUIDraw() override;

	private:

		// drawing and logic of main menu
		void DisplayMainMenu();

	private:

		Camera* mCamera;
		Grid* mGrid;
		
		bool mCheckboxGrid = true;
	};
}