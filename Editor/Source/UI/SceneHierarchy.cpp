#include "SceneHierarchy.h"

#include "Renderer/Gizmo.h"

namespace Cosmos
{
	SceneHierarchy::SceneHierarchy(std::shared_ptr<Scene>& scene, Gizmo* gizmo)
		: Widget("UI:Scene Hierarchy"), mScene(scene), mGizmo(gizmo)
	{
		Logger() << "Creating Scene Hierarchy";
	}

	SceneHierarchy::~SceneHierarchy()
	{
	}

	void SceneHierarchy::OnUpdateUI()
	{
		ImGuiWindowFlags flags{};
		flags |= ImGuiWindowFlags_HorizontalScrollbar;

		ImGui::Begin("Scene Hierarchy", 0, flags);

		for (size_t i = 0; i < mScene->Entities().Entities().size(); i++)
		{
			std::ostringstream entName;
			entName << mScene->Entities().Entities()[i]->Name() << " ";
			entName << mScene->Entities().Entities()[i]->ID();

			if (ImGui::TreeNodeEx(entName.str().c_str()))
			{
				ImGui::TreePop();
			}
		}

		ImGui::End();

		//ImGui::ShowDemoWindow();
	}
}