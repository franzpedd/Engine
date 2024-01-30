#include "epch.h"
#include "Scene.h"

#include "Entity/Entity.h"
#include "Entity/Camera.h"
#include "Renderer/Renderer.h"
#include "Platform/Window.h"

#include <iostream>

namespace Cosmos
{
	Scene::Scene(std::shared_ptr<Window>& window)
		: mWindow(window)
	{
		Logger() << "Creating Scene";

		mEntities = EntityStack::Create();
	}

	void Scene::CreateEntity(const char* name)
	{
		UUID id = UUID();
		entt::entity entt = mRegistry.create();

		Entity entity = { this, entt };
		entity.AddComponent<IDComponent>(id);
		entity.AddComponent<NameComponent>(name);
		
		mEntityMap[id] = entity;
	}

	void Scene::DestroyEntity(Entity* entity)
	{
		if (entity == nullptr)
			return;

		if (entity->HasComponent<ModelComponent>())
			entity->GetComponent<ModelComponent>().model->Destroy();

		mEntityMap.erase(entity->GetUUID());
	}

	Entity* Scene::FindEntityById(UUID id)
	{
		Entity* found = nullptr;
		auto it = mEntityMap.find(id);
		
		if (it != mEntityMap.end())
			found = &(it->second);
		
		return found;
	}

	void Scene::OnUpdate(float timestep)
	{
		PROFILER_FUNCTION();

		// special entities (editor entities like grid and viewport)
		for (auto& ent : mEntities->GetEntitiesVector())
		{
			ent->OnUpdate(timestep);
		}

		auto modelsGroup = mRegistry.group<ModelComponent>();
		for (auto ent : modelsGroup)
		{
			auto& [model] = modelsGroup.get<ModelComponent>(ent);

			if (model == nullptr || !model->IsLoaded())
				continue;

			model->Update(timestep);
		}
	}

	void Scene::OnRenderDraw()
	{
		for (auto& ent : mEntities->GetEntitiesVector())
		{
			ent->OnRenderDraw();
		}

		uint32_t currentFrame = mRenderer->CurrentFrame();
		VkDeviceSize offsets[] = { 0 };
		VkCommandBuffer commandBuffer = mRenderer->GetCommander().AccessMainCommandEntry()->commandBuffers[currentFrame];

		auto modelsGroup = mRegistry.group<ModelComponent>();
		for (auto ent : modelsGroup)
		{
			auto& [model] = modelsGroup.get<ModelComponent>(ent);
		
			if (model == nullptr|| !model->IsLoaded())
				continue;
		
			model->Draw(commandBuffer);
		}
	}

	void Scene::Destroy()
	{
		for (auto& ent : mEntities->GetEntitiesVector())
		{
			ent->OnDestroy();
		}

		for (auto& ent : mEntityMap)
		{
			auto entity = &ent.second;

			// destroy all components before cleaning the entitymap
			if (entity->HasComponent<ModelComponent>() && entity->GetComponent<ModelComponent>().model != nullptr)
			{
				if (entity->GetComponent<ModelComponent>().model->IsLoaded())
					entity->GetComponent<ModelComponent>().model->Destroy();

				delete entity->GetComponent<ModelComponent>().model;
			}
		}

		mEntityMap.clear();
	}

	void Scene::Load(Serializer& sceneEntities)
	{
		LOG_TO_TERMINAL(Logger::Warn, "Loading many scenes may result in UUID conflicts, must reset UUID on loading scenes");

		for (const auto entity : mRegistry.view<IDComponent>())
		{
			mRegistry.destroy(entity);
		}

		mEntityMap.clear();

		std::map<std::string, Serializer> map;
		sceneEntities["Entities"].get_to(map);

		for (auto& entity : map)
		{
			entt::entity entt = mRegistry.create();
			UUID id = UUID();
			Entity newEntity = { this, entt };

			newEntity.AddComponent<IDComponent>(id);

			for (auto& component : entity.second.items())
			{
				try
				{
					std::string field = component.value().at(0);

					if (field.compare("Name") == 0)
					{
						newEntity.AddComponent<NameComponent>(component.value().at(1));
					}

					if (field.compare("Transform") == 0)
					{
						newEntity.AddComponent<TransformComponent>();
						auto& entComponent = newEntity.GetComponent<TransformComponent>();

						uint32_t index = 0;
						for (auto& item : component.value().at(1))
						{
							glm::vec3 values = glm::vec3(0.0f);
							item[0].get_to(values.x);
							item[1].get_to(values.y);
							item[2].get_to(values.z);

							switch (index)
							{
							case 0: entComponent.rotation = values; break;
							case 1: entComponent.scale = values; break;
							case 2: entComponent.translation = values; break;
							default: break;
							}

							index++;
						}
					}
				}

				catch (const Serializer::exception& e)
				{
					std::cout << e.what() << std::endl;
				}
			}
			mEntityMap[id] = newEntity;
		}
	}

	Serializer Scene::Serialize()
	{
		Serializer save;

		for (auto& entid : mEntityMap)
		{
			// entity name
			std::string uuid = entid.first;

			Entity entity{ this, entid.second.GetHandle() };
			save["Entities"][uuid].push_back({"Name", entity.GetComponent<NameComponent>().name});

			if (entity.HasComponent<TransformComponent>())
			{
				auto& component = entity.GetComponent<TransformComponent>();
				
				Serializer transform;
				transform["Translation"] = { component.translation.x, component.translation.y, component.translation.z};
				transform["Rotation"] = { component.rotation.x, component.rotation.y, component.rotation.z };
				transform["Scale"] = { component.scale.x, component.scale.y, component.scale.z };
				save["Entities"][uuid].push_back({ "Transform", transform });
			}
		}

		return save;
	}
}