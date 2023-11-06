#include "SceneHierarchy.h"

#include "Renderer/Gizmo.h"

namespace Cosmos
{
	SceneHierarchy::SceneHierarchy(Scene* scene, Gizmo* gizmo)
		: Widget("UI:Scene Hierarchy"), mScene(scene), mGizmo(gizmo)
	{
		Logger() << "Creating Scene Hierarchy";
	}

	SceneHierarchy::~SceneHierarchy()
	{
	}

	void SceneHierarchy::OnUpdateUI()
	{
		DisplaySceneHierarchy();
		DisplaySelectedEntityComponents();
	}

	void SceneHierarchy::DisplaySceneHierarchy()
	{
		ImGuiWindowFlags flags = {};
		flags |= ImGuiWindowFlags_HorizontalScrollbar;
		flags |= ImGuiWindowFlags_MenuBar;

		ImGui::Begin("Scene Hierarchy", 0, flags);

		// displays edit menu
		DisplayEditMenubar();
		
		// draws all existing entity nodes
		{	
			for (auto& entid : mScene->Registry().storage<entt::entity>())
			{
				Entity entity{ mScene, entid };
				DrawEntityNode(entity);
			}
		}

		// draws the components out of the selected entity
		DrawComponents(mSelectedEntity);

		ImGui::End();
	}

	void SceneHierarchy::DisplayEditMenubar()
	{
		if (ImGui::BeginMenuBar())
		{
			float itemSize = 20.0f;
			float itemCount = 2.0;
		
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - itemSize * itemCount);
			itemCount--;
		
			if (ImGui::MenuItem(ICON_FA_PLUS_SQUARE))
			{
				mScene->CreateEntity();
			}
			
			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - itemSize * itemCount);
			itemCount--;
			
			if (ImGui::MenuItem(ICON_FA_MINUS_SQUARE))
			{
				if (mSelectedEntity)
				{
					mScene->DestroyEntity(mSelectedEntity);
					mSelectedEntity = {};
				}
			}
			
			ImGui::EndMenuBar();
		}
	}

	void SceneHierarchy::DisplaySelectedEntityComponents()
	{
		ImGui::Begin("Properties");

		if (mSelectedEntity != nullptr)
		{
			DrawComponents(mSelectedEntity);
		}

		ImGui::End();
	}

	void SceneHierarchy::DrawEntityNode(Entity entity)
	{
		if (!entity.HasComponent<NameComponent>() || !entity.HasComponent<IDComponent>()) return;

		// entity name
		const char* name = entity.GetComponent<NameComponent>().name;

		// window behavior
		ImGuiTreeNodeFlags windowFlags = {};
		windowFlags |= ((mSelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
		
		// tree-node name
		if (ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, windowFlags, name))
		{
			if (ImGui::IsItemClicked())
			{
				mSelectedEntity = entity;
			}

			ImGui::TreePop();
		}
	}

	void SceneHierarchy::DrawComponents(Entity entity)
	{

	}

	template<typename T, typename F>
	void SceneHierarchy::DrawSingleComponent(const char* name, Entity* entity, F function)
	{

	}

	template<typename T>
	void SceneHierarchy::DisplayAddComponentEntry(const char* name)
	{

	}
}