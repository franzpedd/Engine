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

	void Scene::ConnectCamera(Camera* camera)
	{
		mCamera = camera;
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
		{
			if (entity->GetComponent<ModelComponent>().model != nullptr)
			{
				entity->GetComponent<ModelComponent>().model->Destroy();
				entity->GetComponent<ModelComponent>().model->SetLoaded(false);
			}
		}

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

		auto modelsGroup = mRegistry.group<TransformComponent>(entt::get<ModelComponent>);
		for (auto ent : modelsGroup)
		{
			auto& [transformComponent, modelComponent] = modelsGroup.get<TransformComponent, ModelComponent>(ent);

			if (modelComponent.model == nullptr || !modelComponent.model->IsLoaded())
				continue;

			modelComponent.model->Update(timestep, transformComponent.GetTransform());
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

		//auto modelsGroup = mRegistry.group<ModelComponent>();
		//for (auto ent : modelsGroup)
		//{
		//	auto& [model] = modelsGroup.get<ModelComponent>(ent);
		//
		//	if (model == nullptr|| !model->IsLoaded())
		//		continue;
		//
		//	model->Draw(commandBuffer);
		//}
	}

	void Scene::Destroy()
	{
		for (auto& ent : mEntities->GetEntitiesVector())
		{
			ent->OnDestroy();
		}

		CleanCurrentScene();
	}

	void Scene::CleanCurrentScene()
	{
		//for (auto& ent : mEntityMap)
		//{
		//	auto entity = &ent.second;
		//
		//	// the model is a special pointer, call it's destruction methods before deleting it
		//	if (entity->HasComponent<ModelComponent>() && entity->GetComponent<ModelComponent>().model != nullptr)
		//	{
		//		if (entity->GetComponent<ModelComponent>().model->IsLoaded())
		//		{
		//			entity->GetComponent<ModelComponent>().model->Destroy();
		//		}
		//
		//		delete entity->GetComponent<ModelComponent>().model;
		//	}
		//}

		//for (const auto entity : mRegistry.view<IDComponent>())
		//{
		//	mRegistry.destroy(entity);
		//}

		//for (auto& ent : mEntityMap)
		//{
		//	auto entity = &ent.second;
		//
		//	// remove the transform component
		//	if (entity->HasComponent<TransformComponent>())
		//		entity->RemoveComponent<TransformComponent>();
		//
		//	// remove the model component
		//	if (entity->HasComponent<ModelComponent>())
		//	{
		//		if (entity->GetComponent<ModelComponent>().model != nullptr)
		//		{
		//			entity->GetComponent<ModelComponent>().model->Destroy();
		//		}
		//		entity->RemoveComponent<ModelComponent>();
		//	}
		//		
		//}

		for (auto& entRef : mEntityMap)
		{
			auto& entity = entRef.second;

			if (entity.HasComponent<ModelComponent>())
			{
				if (entity.GetComponent<ModelComponent>().model != nullptr)
				{
					entity.GetComponent<ModelComponent>().model->Destroy();
					entity.GetComponent<ModelComponent>().model->SetLoaded(false);
				}
			}

			mEntityMap.erase(entity.GetUUID());
		}
		
		int x = 0;

		//mRegistry.clear();
		//mEntityMap.clear();
	}

	void Scene::Load(DataFile& sceneEntities)
	{
		LOG_TO_TERMINAL(Logger::Warn, "Loading many scenes may result in UUID conflicts, must reset UUID on loading scenes");

		//for (const auto entity : mRegistry.view<IDComponent>())
		//{
		//	mRegistry.destroy(entity);
		//}
		//
		//mEntityMap.clear();
		//
		//std::unordered_map<std::string, OrderedSerializer> unorderedMap;
		//sceneEntities["Entities"].get_to(unorderedMap);
		//
		//for (auto& entity : unorderedMap)
		//{
		//	entt::entity entt = mRegistry.create();
		//	UUID id = UUID();
		//	Entity newEntity = { this, entt };
		//
		//	try
		//	{
		//		for (auto& component : entity.second.items())
		//		{
		//			std::string componentName = component.value().at(0);
		//			
		//			if (componentName.compare("Name") == 0)
		//			{
		//				newEntity.AddComponent<NameComponent>(component.value().at(1));
		//			}
		//
		//			if (componentName.compare("Transform") == 0)
		//			{
		//				newEntity.AddComponent<TransformComponent>();
		//				auto& entComponent = newEntity.GetComponent<TransformComponent>();
		//				
		//				uint32_t index = 0;
		//				for (auto& item : component.value().at(1))
		//				{
		//					glm::vec3 values = glm::vec3(0.0f);
		//					item[0].get_to(values.x);
		//					item[1].get_to(values.y);
		//					item[2].get_to(values.z);
		//					
		//					switch (index)
		//					{
		//						case 0: entComponent.translation = values; break;
		//						case 1: entComponent.rotation = values; break;
		//						case 2: entComponent.scale = values; break;	
		//						default: break;
		//					}
		//					
		//					index++;
		//				}
		//			}
		//
		//			if (componentName.compare("Model") == 0)
		//			{
		//				newEntity.AddComponent<ModelComponent>();
		//				auto& entComponent = newEntity.GetComponent<ModelComponent>();
		//				entComponent.model = new Model(mRenderer, *mCamera);
		//				
		//				for (auto& item : component.value().at(1))
		//				{
		//					std::cout << item << std::endl;
		//				}
		//				
		//				//std::string modelPath;
		//				//serial["Model"].get_to<std::string>(modelPath);
		//				//
		//				//std::string albedoPath;
		//				//serial["Albedo"].get_to<std::string>(albedoPath);
		//				
		//				
		//			}
		//		}
		//
		//	}
		//
		//	catch (const std::exception& e)
		//	{
		//		std::cout << e.what() << std::endl;
		//	}
		//
		//	mEntityMap[id] = newEntity;
		//}
	}

	DataFile Scene::Serialize()
	{
		DataFile save;

		for (auto& entid : mEntityMap)
		{
			Entity& entity = entid.second;

			// discard entities that doesnt have id's (they may be exclusively entities like the Editor Grid)
			if (!entity.HasComponent<IDComponent>() || !entity.HasComponent<NameComponent>())
				continue;

			// write name and id components, they should exists by default
			std::string uuidComponent = entity.GetComponent<IDComponent>().id;
			std::string nameComponent = entity.GetComponent<NameComponent>().name;

			save["Entities"][uuidComponent]["Id"].SetString(uuidComponent);
			save["Entities"][uuidComponent]["Name"].SetString(nameComponent);

			// write the transform component if it exists
			if (entity.HasComponent<TransformComponent>())
			{
				auto& component = entity.GetComponent<TransformComponent>();
				auto& place = save["Entities"][uuidComponent]["Transform"];

				place["Translation"]["X"].SetDouble(component.translation.x);
				place["Translation"]["Y"].SetDouble(component.translation.y);
				place["Translation"]["Z"].SetDouble(component.translation.z);
				
				place["Rotation"]["X"].SetDouble(component.rotation.x);
				place["Rotation"]["Y"].SetDouble(component.rotation.y);
				place["Rotation"]["Z"].SetDouble(component.rotation.z);
				
				place["Scale"]["X"].SetDouble(component.scale.x);
				place["Scale"]["Y"].SetDouble(component.scale.y);
				place["Scale"]["Z"].SetDouble(component.scale.z);
			}

			// write the model component if it exists
			if (entity.HasComponent<ModelComponent>())
			{
				auto& component = entity.GetComponent<ModelComponent>();
				auto& place = save["Entities"][uuidComponent]["Model"];

				place["Path"].SetString(component.model->GetPath());
				place["Albedo"].SetString(component.model->GetAlbedoPath());
			}
		}

		return save;
	}
}