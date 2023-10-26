#include "Mainmenu.h"

namespace Cosmos
{
	Mainmenu::Mainmenu(Camera& camera)
		: mCamera(camera)
	{
		Logger() << "Creating Mainmenu";
	}

	Mainmenu::~Mainmenu()
	{

	}

	void Mainmenu::OnUpdate()
	{
		ImGui::BeginMainMenuBar();
		DisplayMainMenu();
		ImGui::EndMainMenuBar();

		ImGuiIO& io = ImGui::GetIO();

		ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoTitleBar);
		ImGui::Text(ICON_FA_INFO_CIRCLE " FPS: %d", Application::Get()->GetCurrentAverageFPS());
		ImGui::Text(ICON_FA_INFO_CIRCLE " Timestep: %f", Application::Get()->GetTimestep());
		ImGui::Text(ICON_FA_CAMERA " Camera Pos: %.2f %.2f %.2f", mCamera.GetPosition().x, mCamera.GetPosition().y, mCamera.GetPosition().z);
		ImGui::Text(ICON_FA_CAMERA " Camera Rot: %.2f %.2f %.2f", mCamera.GetRotation().x, mCamera.GetRotation().y, mCamera.GetRotation().z);
		
		ImGui::End();

		//ImGui::ShowDemoWindow();
	}

	void Mainmenu::OnResize()
	{
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