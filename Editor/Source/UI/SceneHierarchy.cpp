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

	void SceneHierarchy::OnUpdate()
	{
		DisplaySceneHierarchy();
		DisplaySelectedEntityComponents();

		//ImGui::ShowDemoWindow();
	}

	void SceneHierarchy::DisplaySceneHierarchy()
	{
		ImGuiWindowFlags flags = ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar | ImGuiTreeNodeFlags_OpenOnArrow;
		
		ImGui::Begin("Objects", 0, flags);

		// displays edit menu
		if (ImGui::BeginMenuBar())
		{
			ImGui::BeginGroup();
			{
				ImGui::Text(ICON_FA_PAINT_BRUSH " Edit Entity");

				float itemSize = 35.0f;
				ImGui::SetCursorPosX(ImGui::GetWindowWidth() - itemSize);
				
				if (ImGui::MenuItem(ICON_FA_PLUS_SQUARE))
				{
					mScene->CreateEntity();
				}
			}
			ImGui::EndGroup();
		}

		ImGui::EndMenuBar();
		
		// draws all existing entity nodes
		for (auto& entid : mScene->GetEntityMap())
		{
			Entity entity{ mScene, entid.second };
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

			float itemSize = 35.0f;
			float itemCount = 1.0f;

			ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (itemSize * itemCount));
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
		windowFlags |= ((mSelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanAvailWidth;
		
		// tree-node name
		if (ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, windowFlags, name.c_str()))
		{
			if (ImGui::IsItemClicked())
			{
				mSelectedEntity = entity;
			}

			if (ImGui::BeginPopupContextItem("##RightClickEntity"))
			{
				if (ImGui::MenuItem("Remove Entity"))
				{
					if (mSelectedEntity)
					{
						mScene->DestroyEntity(mSelectedEntity);
						mSelectedEntity = {};
					}
				}
			
				ImGui::EndPopup();
			}

			ImGui::TreePop();
		}
	}

	void SceneHierarchy::DrawComponents(Entity entity)
	{
		// id and name (general info)
		{
			ImGui::Separator();

			{
				uint64_t id = entity.GetComponent<IDComponent>().id;
				std::string idStr = "ID: ";
				idStr.append(std::to_string(id));

				ImGui::Text("%s", idStr.c_str());
			}

			ImGui::Text("Name: ");
			ImGui::SameLine();

			{
				auto& name = entity.GetComponent<NameComponent>().name;
				char buffer[ENTITY_NAME_MAX_CHARS];
				memset(buffer, 0, sizeof(buffer));
				strncpy_s(buffer, sizeof(buffer), name.c_str(), sizeof(buffer));

				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 2.0f));

				if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
				{
					name = std::string(buffer);
				}

				ImGui::PopStyleVar();
			}

			ImGui::Separator();
		}

		DrawComponent<TransformComponent>("Transform", mSelectedEntity, [](TransformComponent& component)
			{
				ImGui::Text("T: ");
				ImGui::SameLine();
				Vector3Control("Translation", component.translation);

				ImGui::Text("R: ");
				ImGui::SameLine();
				Vector3Control("Rotation", component.rotation);

				ImGui::Text("S: ");
				ImGui::SameLine();
				Vector3Control("Scale", component.scale);

			});

		// Sprite
		//DrawComponent<SpriteComponent>("Sprite", mSelectedEntity, [](SpriteComponent& component)
		//	{
		//
		//	});
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

			if (ImGui::BeginPopupContextItem("##RightClickComponent"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					entity.RemoveComponent<T>();
				}

				ImGui::EndPopup();
			}
		}
	}
}