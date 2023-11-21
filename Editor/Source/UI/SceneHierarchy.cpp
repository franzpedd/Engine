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

		ImGui::ShowDemoWindow();
	}

	void SceneHierarchy::DisplaySceneHierarchy()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar | ImGuiTreeNodeFlags_OpenOnArrow;
		
		ImGui::Begin("Objects", 0, flags);

		// displays edit menu
		if (ImGui::BeginMenuBar())
		{
			ImGui::Text(ICON_FA_PAINT_BRUSH " Edit Entity");

			float itemSize = 20.0f;
			float itemCount = 2.0f;

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
		
		// draws all existing entity nodes
		for (auto& entid : mScene->Registry().storage<entt::entity>())
		{
			Entity entity{ mScene, entid };
			DrawEntityNode(entity);
		}
		
		ImGui::End();
	}

	void SceneHierarchy::DisplaySelectedEntityComponents()
	{
		ImGuiWindowFlags flags = {};
		flags |= ImGuiWindowFlags_HorizontalScrollbar;
		flags |= ImGuiWindowFlags_MenuBar;
		
		ImGui::Begin("Components", 0, flags);

		if (!mSelectedEntity)
		{
			ImGui::End();
			return;
		}
		
		// dispaly edit menu
		if (ImGui::BeginMenuBar())
		{
			ImGui::Text(ICON_FA_PAINT_BRUSH " Edit Components");

			float itemSize = 20.0f;
			float itemCount = 1.0f;

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - itemSize * itemCount);
			itemCount--;

			if (ImGui::BeginMenu(ICON_FA_PLUS_SQUARE))
			{
				DisplayAddComponentEntry<TransformComponent>("Transform");

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		
		if (mSelectedEntity)
		{
			DrawComponents(mSelectedEntity);
		}
		
		ImGui::End();
	}

	void SceneHierarchy::DrawEntityNode(Entity entity)
	{
		if (!entity.HasComponent<NameComponent>() || !entity.HasComponent<IDComponent>()) return;
		
		// entity name
		std::string name = entity.GetComponent<NameComponent>().name;
		
		// window behavior
		ImGuiTreeNodeFlags windowFlags = {};
		windowFlags |= ((mSelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanAvailWidth;
		
		// tree-node name
		if (ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, windowFlags, name.c_str()))
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
		// id
		{
			if (ImGui::TreeNodeEx("##ID", 0, "ID"))
			{
				uint64_t id = entity.GetComponent<IDComponent>().id;
				ImGui::Text("%d", id);

				ImGui::TreePop();
			}
		}
		
		// Name
		{
			auto& name = entity.GetComponent<NameComponent>().name;
			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), name.c_str(), sizeof(buffer));
		
			if (ImGui::TreeNodeEx("##Name", 0, "Name"))
			{
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 2.0f));

				if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				{
					name = std::string(buffer);
				}
				ImGui::PopStyleVar();
				ImGui::TreePop();
			}
		}
		

		// 3D Transformation matrix
		DrawComponent<TransformComponent>("Transform", mSelectedEntity, [](auto& component)
			{

			});
	}

	template<typename T>
	void SceneHierarchy::DisplayAddComponentEntry(const char* name)
	{
		if (ImGui::MenuItem(name))
		{
			if (!mSelectedEntity.HasComponent<T>())
			{
				mSelectedEntity.AddComponent<T>();
				return;
			}
			
			LOG_TO_TERMINAL(Logger::Severity::Warn, "Entity %s already have the component %s", mSelectedEntity.GetComponent<NameComponent>().name.c_str(), name);
		}
	}

	template<typename T, typename F>
	static void SceneHierarchy::DrawComponent(const char* name, Entity& entity, F func)
	{
		if (entity.HasComponent<T>())
		{
			auto& component = entity.GetComponent<T>();
			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), 0, name))
			{
				func(component);
				ImGui::TreePop();
			}
		}
	}
}