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

	void Scene::DestroyEntity(Entity entity)
	{
		mEntityMap.erase(entity.GetUUID());
		mRegistry.destroy(entity);
	}

	Entity Scene::FindEntityByName(const char* name)
	{
		auto view = mRegistry.view<NameComponent>();
		
		for (auto entity : view)
		{
			const NameComponent& nc = view.get<NameComponent>(entity);
		
			if (nc.name == name)
			{
				return Entity{ this, entity };
			}
		}

		LOG_TO_TERMINAL(Logger::Severity::Warn, "Could not find any entity with given name");
		return nullptr;
	}

	Entity Scene::FindEntityById(UUID id)
	{
		if (mEntityMap.find(id) != mEntityMap.end())
		{
			return { this, mEntityMap[id] };
		}

		LOG_TO_TERMINAL(Logger::Severity::Warn, "Could not find any entity with given id");
		return nullptr;
	}

	void Scene::OnUpdate(float timestep)
	{
		PROFILER_FUNCTION();

		// discard this update function and perform updating on components
		for (auto& ent : mEntities->GetEntitiesVector())
		{
			ent->OnUpdate(timestep);
		}

		//{ // example on how to iterate on components
		//	auto group = m_Registry.group<TransformComponent>(entt::get<SpriteRendererComponent>);
		//	for (auto entity : group)
		//	{
		//		auto [transform, sprite] = group.get<TransformComponent, SpriteRendererComponent>(entity);
		//
		//		Renderer2D::DrawSprite(transform.GetTransform(), sprite, (int)entity);
		//	}
		//}
	}

	void Scene::OnRenderDraw()
	{
		for (auto& ent : mEntities->GetEntitiesVector())
		{
			ent->OnRenderDraw();
		}
	}

	void Scene::Destroy()
	{
		for (auto& ent : mEntities->GetEntitiesVector())
		{
			ent->OnDestroy();
		}
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
				std::cout << component.key() << " : " << component.value() << "\n";

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
			std::string uuid = std::to_string(entid.first);

			Entity entity{ this, entid.second };
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