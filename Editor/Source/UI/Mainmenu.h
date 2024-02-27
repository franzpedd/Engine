#pragma once

#include <Engine.h>

#include "Core/Project.h"

namespace Cosmos
{
	// forward declarations
	class Grid;
	class SceneHierarchy;

	class Mainmenu : public Widget
	{
	public:

		enum Action
		{
			None = 0,
			New, Open, Save, SaveAs, Undo, Redo
		};

	public:

		// constructor
		Mainmenu(std::unique_ptr<Project>& project, Grid* grid, SceneHierarchy* sceneHierarchy);

		// destructor
		virtual ~Mainmenu() = default;

	public:

		// updates the ui element
		virtual void OnUpdate() override;

	private:

		// drawing and logic of main menu
		void DisplayMainMenu();

		// handles selected menu option
		void HandleMenuAction();

	private:

		std::unique_ptr<Project>& mProject;
		Grid* mGrid;
		SceneHierarchy* mSceneHierarchy;
		
		bool mCheckboxGrid = true;
		Action mMenuAction = Action::None;
		bool mCancelAction = false;
	};
}