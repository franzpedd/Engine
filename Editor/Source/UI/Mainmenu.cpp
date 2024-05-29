#include "Mainmenu.h"

#include "SceneHierarchy.h"
#include "Renderer/Grid.h"

namespace Cosmos
{
	Mainmenu::Mainmenu(std::unique_ptr<Project>& project, Grid* grid, SceneHierarchy* sceneHierarchy)
		: Widget("UI:Mainmenu"), mProject(project), mGrid(grid), mSceneHierarchy(sceneHierarchy)
	{
		Logger() << "Creating Mainmenu";

		Shared<VKDevice> device = std::dynamic_pointer_cast<VKRenderer>(Application::GetInstance()->GetRenderer())->GetDevice();
		Scene* scene = Application::GetInstance()->GetActiveScene();

		for(uint8_t i = 0; i < 6; i++)
		{
			mSkyboxImages[i].texture = Texture2D::Create(device, scene->GetSkybox()->GetPathsRef()[i].c_str());
			mSkyboxImages[i].descriptor = AddTexture(mSkyboxImages[i].texture->GetSampler(), mSkyboxImages[i].texture->GetView());
		}
	}

	void Mainmenu::OnUpdate()
	{
		ImGui::BeginMainMenuBar();
		DisplayMainMenu();
		ImGui::EndMainMenuBar();

		HandleMenuAction();

		// scene info
		ImGuiWindowFlags flags = {};

		auto camera = Application::GetInstance()->GetCamera();

		ImGui::Begin("Info", nullptr, flags);
		ImGui::Text(ICON_FA_INFO_CIRCLE " FPS: %d", Application::GetInstance()->GetFPSSystem()->GetFPS());
		ImGui::Text(ICON_FA_INFO_CIRCLE " Timestep: %f", Application::GetInstance()->GetFPSSystem()->GetTimestep());
		ImGui::Text(ICON_FA_CAMERA " Camera Pos: %.2f %.2f %.2f", camera->GetPositionRef().x, camera->GetPositionRef().y, camera->GetPositionRef().z);
		ImGui::Text(ICON_FA_CAMERA " Camera Rot: %.2f %.2f %.2f", camera->GetRotationRef().x, camera->GetRotationRef().y, camera->GetRotationRef().z);

		ImGui::End();

		// scene settings
		SceneSettingsWindow();
	}

	void Mainmenu::DisplayMainMenu()
	{
		mMenuAction = Action::None;

		if (ImGui::BeginMenu(ICON_FA_FILE " Project"))
		{
			if (ImGui::MenuItem("New")) mMenuAction = Action::New;
			if (ImGui::MenuItem("Open")) mMenuAction = Action::Open;
			if (ImGui::MenuItem("Save")) mMenuAction = Action::Save;
		
			ImGui::Separator();
		
			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_FA_BOOKMARK " View"))
		{
			if (CheckboxSliderEx("Draw Grid", &mCheckboxGrid))
			{
				mGrid->ToogleOnOff();
			}

			ImGui::EndMenu();
		}

		if (ImGui::BeginMenu(ICON_FA_COG " Settings"))
		{
			if (ImGui::MenuItem("Scene Settings", nullptr))
			{
				mDisplaySceneSettings = true;
			}

			ImGui::EndMenu();
		}
	}

	void Mainmenu::HandleMenuAction()
	{
		switch (mMenuAction)
		{			
			case Cosmos::Mainmenu::New:
			{
				mProject->New();
				mSceneHierarchy->UnselectEntity();
				break;
			}

			case Cosmos::Mainmenu::Open:
			{
				mProject->Open();
				mSceneHierarchy->UnselectEntity();
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
		}
	}

	void Mainmenu::SceneSettingsWindow()
	{
		if (!mDisplaySceneSettings)
			return;

		ImGuiWindowFlags flags = {};
		if (!ImGui::Begin("Scene Settings", &mDisplaySceneSettings, flags))
		{ 
			ImGui::End();
		}

		else
		{
			ImGui::SeparatorText("Skybox configuration");
			ImGui::TextDisabled("Drag and drop from Explorer");

			const char* sides[6] = { "Right", "Left", "Top", "Bottom", "Front", "Back" };
			for(uint8_t i = 0; i < 6; i++)
			{
				if(UI::ImageBrowser(sides[i], mSkyboxImages[i].descriptor, ImVec2(64.0f, 64.0f)))
				{
					// attach new texture for right image
				}

				ImGui::SameLine();
			}

			if(ImGui::Button("Rebuild"))
			{
				// rebuilds the skybox, possibly with new faces
			}

			ImGui::End();
		}
	}
}