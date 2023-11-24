#include "Mainmenu.h"

#include "Renderer/Grid.h"

namespace Cosmos
{
	Mainmenu::Mainmenu(Camera* camera, Grid* grid)
		: Widget("UI:Mainmenu"), mCamera(camera), mGrid(grid)
	{
		Logger() << "Creating Mainmenu";
	}

	void Mainmenu::OnUpdate()
	{
		ImGui::BeginMainMenuBar();
		DisplayMainMenu();
		ImGui::EndMainMenuBar();

		ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(ICON_FA_INFO_CIRCLE " FPS: %d", Application::Get()->GetAverageFPS());
		ImGui::Text(ICON_FA_INFO_CIRCLE " Timestep: %f", Application::Get()->GetTimeStep());
		ImGui::Text(ICON_FA_CAMERA " Camera Pos: %.2f %.2f %.2f", mCamera->GetPosition().x, mCamera->GetPosition().y, mCamera->GetPosition().z);
		ImGui::Text(ICON_FA_CAMERA " Camera Rot: %.2f %.2f %.2f", mCamera->GetRotation().x, mCamera->GetRotation().y, mCamera->GetRotation().z);
		
		ImGui::End();
	}

	void Mainmenu::DisplayMainMenu()
	{
		if (ImGui::BeginMenu(ICON_FA_FILE " Project"))
		{
			if(ImGui::MenuItem(ICON_FA_UNDO " Undo"))
			{

			}

			if (ImGui::MenuItem(ICON_FA_FAST_FORWARD " Redo"))
			{

			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_FA_BOOKMARK "View"))
		{
			if (CheckboxSliderEx("Draw Grid", &mCheckboxGrid))
			{
				mGrid->ToogleOnOff();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_FA_QUESTION_CIRCLE " Help"))
		{
			if (ImGui::MenuItem(ICON_FA_COG " Engine Settings"))
			{

			}

			if (ImGui::MenuItem(ICON_FA_COG " Editor Settings"))
			{

			}
			
			ImGui::EndMenu();
		}

		if (ImGui::Button(ICON_FA_PLAY_CIRCLE))
		{

		}

		if (ImGui::Button(ICON_FA_PAUSE_CIRCLE))
		{

		}

		if (ImGui::Button(ICON_FA_STOP_CIRCLE))
		{

		}
	}
}