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
			New,
			Open,
			Save,
			SaveAs
		};

		struct AssetResource
		{
			Shared<Texture2D> texture;
			VkDescriptorSet descriptor = VK_NULL_HANDLE;
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

		// draws teh scene settings
		void SceneSettingsWindow();

	private:

		std::unique_ptr<Project>& mProject;
		Grid* mGrid;
		SceneHierarchy* mSceneHierarchy;
		
		bool mCheckboxGrid = true;
		Action mMenuAction = Action::None;
		bool mCancelAction = false;

		bool mDisplaySceneSettings = true;

		std::array<AssetResource, 6> mSkyboxImages;
	};
}