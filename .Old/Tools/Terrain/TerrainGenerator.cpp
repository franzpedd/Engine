#include "TerrainGenerator.h"

namespace Cosmos::tools
{
	TerrainGenerator::TerrainGenerator(std::shared_ptr<Renderer> renderer, Scene* scene)
		: mRenderer(renderer), mScene(scene)
	{

	}

	TerrainGenerator::~TerrainGenerator()
	{
	}

	void TerrainGenerator::OnUpdate()
	{
		if (mDisplayGeneratorMenu)
			DrawGeneratorMenu();
	}

	bool TerrainGenerator::DisplayGeneratorMenu(bool value)
	{
		return mDisplayGeneratorMenu = value;
	}

	void TerrainGenerator::DrawGeneratorMenu()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_NoDocking;

		if (ImGui::Begin(ICON_LC_BLEND " Terrain Generator", &mDisplayGeneratorMenu, flags))
		{
			constexpr int maxDragValue = 65536;

			ImGui::Separator();

			ImGui::Text(ICON_LC_ARROW_RIGHT);
			ImGui::SameLine();
			ImGui::DragInt("Width", &mWidth, 1.0f, 0, maxDragValue);

			ImGui::Text(ICON_LC_ARROW_UP);
			ImGui::SameLine();
			ImGui::DragInt("Height", &mHeight, 1.0f, 0, maxDragValue);

			ImGui::Text(ICON_LC_TRIANGLE);
			ImGui::SameLine();
			ImGui::DragInt("Divisions", &mDivisions, 1.0f, 0, maxDragValue);

			ImGui::Separator();

			if (ImGui::Button("Generate"))
			{
				CreateTerrainPlane(mWidth, mHeight, mDivisions);
				DisplayGeneratorMenu(false);
			}

			ImGui::Separator();

			// testing
			//static bool selected[3 * 3] = { true, false, true, false, true, false, true, false, true };
			//for (int y = 0; y < 3; y++)
			//{
			//	for (int x = 0; x < 3; x++)
			//	{
			//		ImVec2 alignment = ImVec2((float)x / 2.0f, (float)y / 2.0f);
			//		char name[32];
			//		sprintf(name, "(%.1f,%.1f)", alignment.x, alignment.y);
			//		if (x > 0) ImGui::SameLine();
			//		ImGui::PushStyleVar(ImGuiStyleVar_SelectableTextAlign, alignment);
			//		ImGui::Selectable(name, &selected[3 * y + x], ImGuiSelectableFlags_None, ImVec2(80, 80));
			//		ImGui::PopStyleVar();
			//	}

		}

		ImGui::End();
	}

	void TerrainGenerator::CreateResources()
	{

	}

	void TerrainGenerator::CreateTerrainPlane(int32_t width, int32_t height, int32_t divisions)
	{
		if (width == 0 || height == 0) return;

		//Entity* ent = mScene->CreateEntity("Terrain");
		//ent->AddComponent<TransformComponent>();
		//ent->AddComponent<TerrainComponent>();
		//ent->GetComponent<TerrainComponent>().terrain = std::make_shared<renderable::Terrain>(mRenderer->GetDevice(), width, height, divisions);
	}
}