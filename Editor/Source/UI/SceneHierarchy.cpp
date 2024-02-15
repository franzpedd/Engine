#include "SceneHierarchy.h"

namespace Cosmos
{
	SceneHierarchy::SceneHierarchy(Scene* scene, Camera& camera)
		: Widget("UI:Scene Hierarchy"), mScene(scene), mCamera(camera)
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

	Entity* SceneHierarchy::GetSelectedEntity()
	{
		return mSelectedEntity;
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
		for (auto& ent : mScene->GetEntityMap())
		{
			bool redraw = false;
			DrawEntityNode(&ent.second, &redraw);

			if (redraw)
				break;
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
				DisplayAddComponentEntry<ModelComponent>("Model");

				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}
		
		DrawComponents(mSelectedEntity);
		
		ImGui::End();
	}

	void SceneHierarchy::DrawEntityNode(Entity* entity, bool* redraw)
	{
		if (entity == nullptr)
			return;

		std::string name = entity->GetComponent<NameComponent>().name;

		ImGuiTreeNodeFlags windowFlags = {};
		windowFlags |= ((mSelectedEntity == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_SpanAvailWidth;

		if (ImGui::TreeNodeEx((void*)(uint64_t)entity, windowFlags, name.c_str()))
		{
			mSelectedEntity = entity;

			if (ImGui::BeginPopupContextItem("##RightClickEntity"))
			{
				if (ImGui::MenuItem("Remove Entity"))
				{
					mScene->DestroyEntity(mSelectedEntity);
					mSelectedEntity = nullptr;
					*redraw = true;
				}
				ImGui::EndPopup();
			}
			ImGui::TreePop();
		}
	}

	void SceneHierarchy::DrawComponents(Entity* entity)
	{
		if (entity == nullptr)
			return;

		// id and name (general info)
		{
			ImGui::Separator();

			{
				uint64_t id = entity->GetComponent<IDComponent>().id;
				std::string idStr = "ID: ";
				idStr.append(std::to_string(id));

				ImGui::Text("%s", idStr.c_str());
			}

			ImGui::Text("Name: ");
			ImGui::SameLine();

			{
				auto& name = entity->GetComponent<NameComponent>().name;
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

		// 3d world-position
		DrawComponent<TransformComponent>("Transform", mSelectedEntity, [&](TransformComponent& component)
			{
				ImGui::Text("T: ");
				ImGui::SameLine();
				Vector3Control("Translation", component.translation);

				ImGui::Text("R: ");
				ImGui::SameLine();
				glm::vec3 rotation = glm::degrees(component.rotation);
				Vector3Control("Rotation", rotation);
				component.rotation = glm::radians(rotation);

				ImGui::Text("S: ");
				ImGui::SameLine();
				Vector3Control("Scale", component.scale);
			});

		// 3d geometry
		DrawComponent<ModelComponent>("Model", mSelectedEntity, [&](ModelComponent& component)
			{
				if (!component.model)
					component.model = new Model(mScene->GetRenderer(), mCamera);

				// model path
				{
					ImGui::BeginGroup();

					ImGui::Text(ICON_FA_CUBE " ");
					ImGui::SameLine();

					auto& modelPath = component.model->GetPath();
					char buffer[ENTITY_NAME_MAX_CHARS];
					memset(buffer, 0, sizeof(buffer));
					strncpy_s(buffer, sizeof(buffer), modelPath.c_str(), sizeof(buffer));

					ImGui::InputTextWithHint("", "Drag and drop from Explorer", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);

					ImGui::EndGroup();

					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER"))
						{
							std::filesystem::path path = (const char*)payload->Data;
							component.model->LoadFromFile(path.string());
						}

						ImGui::EndDragDropTarget();
					}
				}

				ImGui::Separator();
				
				// model albedo texture
				{
					ImGui::BeginGroup();
					
					ImGui::Text(ICON_FA_PAINT_BRUSH " ");
					ImGui::SameLine();
					
					auto& texturePath = component.model->GetAlbedoPath();
					char buffer[ENTITY_NAME_MAX_CHARS];
					memset(buffer, 0, sizeof(buffer));
					strncpy_s(buffer, sizeof(buffer), texturePath.c_str(), sizeof(buffer));
					
					//ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5.0f, 2.0f));
					ImGui::InputTextWithHint("", "Drag and drop from Explorer", buffer, sizeof(buffer), ImGuiInputTextFlags_ReadOnly);
					//ImGui::PopStyleVar();
					
					ImGui::EndGroup();
					
					if (ImGui::BeginDragDropTarget())
					{
						if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("EXPLORER"))
						{
							std::filesystem::path path = (const char*)payload->Data;
							component.model->LoadAlbedoTexture(path.string());
						}
					
						ImGui::EndDragDropTarget();
					}
				}
			});
	}

	template<typename T>
	void SceneHierarchy::DisplayAddComponentEntry(const char* name)
	{
		if (ImGui::MenuItem(name))
		{
			if (!mSelectedEntity->HasComponent<T>())
			{
				mSelectedEntity->AddComponent<T>();
				return;
			}
			
			LOG_TO_TERMINAL(Logger::Severity::Warn, "Entity %s already have the component %s", mSelectedEntity->GetComponent<NameComponent>().name.c_str(), name);
		}
	}

	template<typename T, typename F>
	static void SceneHierarchy::DrawComponent(const char* name, Entity* entity, F func)
	{
		if (entity == nullptr)
			return;

		if (entity->HasComponent<T>())
		{
			auto& component = entity->GetComponent<T>();
			if (ImGui::TreeNodeEx((void*)typeid(T).hash_code(), 0, name))
			{
				func(component);
				ImGui::TreePop();
			}

			if (ImGui::BeginPopupContextItem("##RightClickComponent"))
			{
				if (ImGui::MenuItem("Remove Component"))
				{
					entity->RemoveComponent<T>();
				}

				ImGui::EndPopup();
			}
		}
	}
}