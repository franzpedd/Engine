#include "Mainmenu.h"

#include "Renderer/Grid.h"

namespace Cosmos
{
	Mainmenu::Mainmenu(std::unique_ptr<Project>& project, Camera* camera, Grid* grid)
		: Widget("UI:Mainmenu"), mProject(project), mCamera(camera), mGrid(grid)
	{
		Logger() << "Creating Mainmenu";
	}

	void Mainmenu::OnUpdate()
	{
		ImGui::BeginMainMenuBar();
		DisplayMainMenu();
		ImGui::EndMainMenuBar();

		HandleMenuAction();

		ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(ICON_FA_INFO_CIRCLE " FPS: %d", Application::Get()->GetAverageFPS());
		ImGui::Text(ICON_FA_INFO_CIRCLE " Timestep: %f", Application::Get()->GetTimeStep());
		ImGui::Text(ICON_FA_CAMERA " Camera Pos: %.2f %.2f %.2f", mCamera->GetPosition().x, mCamera->GetPosition().y, mCamera->GetPosition().z);
		ImGui::Text(ICON_FA_CAMERA " Camera Rot: %.2f %.2f %.2f", mCamera->GetRotation().x, mCamera->GetRotation().y, mCamera->GetRotation().z);

		ImGui::End();

		ImGui::ShowDemoWindow();
	}

	void Mainmenu::DisplayMainMenu()
	{
		mMenuAction = Action::None;

		if (ImGui::BeginMenu(ICON_FA_FILE " Project"))
		{
			if (ImGui::MenuItem("New")) mMenuAction = Action::New;
			if (ImGui::MenuItem("Open")) mMenuAction = Action::Open;
			if (ImGui::MenuItem("Save")) mMenuAction = Action::Save;
			if (ImGui::MenuItem("Save As")) mMenuAction = Action::SaveAs;
		
			ImGui::Separator();
		
			if (ImGui::MenuItem(ICON_FA_UNDO " Undo")) mMenuAction = Action::Undo;
			if (ImGui::MenuItem(ICON_FA_FAST_FORWARD " Redo")) mMenuAction = Action::Redo;
		
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

	void Mainmenu::HandleMenuAction()
	{
		switch (mMenuAction)
		{
			case Cosmos::Mainmenu::New:
			{
				ImGui::OpenPopup("Save current Project?");
				break;
			}
				
			case Cosmos::Mainmenu::Open:
			{
				mProject->Open();
				break;
			}

			case Cosmos::Mainmenu::Save:
			{
				mProject->Save();
				break;
			}

			case Cosmos::Mainmenu::SaveAs:
			{
				mProject->SaveAs();
				break;
			}

			default:
			{
				break;
			}
		}

		if (ImGui::BeginPopupModal("Save current Project?"))
		{
			ImGui::Text("Do you want to save changes to '%s'", mProject->GetName().c_str());

			if (ImGui::Button("Save"))
			{
				mProject->Save();
			}

			ImGui::SameLine();
			
			if (ImGui::Button("Save As"))
			{
				mProject->SaveAs();
			}

			ImGui::SameLine();
			
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}
		
			ImGui::EndPopup();
		}
	}
}