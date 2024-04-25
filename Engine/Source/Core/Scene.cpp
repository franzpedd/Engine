#include "epch.h"
#include "Scene.h"

#include "Entity/Entity.h"
#include "Renderer/Renderer.h"
#include "Platform/Window.h"
#include "UI/GUI.h"

#include <iostream>

namespace Cosmos
{
	Scene::Scene(Shared<Renderer> renderer)
		: mRenderer(renderer)
	{
		Logger() << "Creating Scene";
	}

	Scene::~Scene()
	{
		mEntityMap.clear();
		mRegistry.clear();
	}

	Entity* Scene::CreateEntity(const char* name)
	{
		UUID id = UUID();
		entt::entity entt = mRegistry.create();

		Entity entity{ shared_from_this(), mRegistry.create(), id };
		entity.AddComponent<NameComponent>(name);
		
		mEntityMap[id] = entity;
		return &mEntityMap[id];
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
			found = &it->second;
		
		return found;
	}

	Entity* Scene::FindEntityByHandle(entt::entity handle)
	{
		Entity* found = nullptr;

		for (auto& entity : mEntityMap)
		{
			if (entity.second.GetHandle() == handle)
				found = &entity.second;
		}

		return found;
	}

	void Scene::OnUpdate(float timestep)
	{
		// update models
		auto modelsGroup = mRegistry.group<TransformComponent>(entt::get<ModelComponent>);
		for (auto ent : modelsGroup)
		{
			auto& [transformComponent, modelComponent] = modelsGroup.get<TransformComponent, ModelComponent>(ent);

			if (modelComponent.model == nullptr || !modelComponent.model->IsLoaded())
				continue;

			modelComponent.model->Update(timestep, transformComponent.GetTransform());
		}
	}

	void Scene::OnRender()
	{
		GUI::Get()->OnRenderDraw();

		uint32_t currentFrame = mRenderer->CurrentFrame();
		VkDeviceSize offsets[] = { 0 };
		VkCommandBuffer commandBuffer = Commander::Get().GetPrimary()->commandBuffers[currentFrame];

		// draw models
		auto modelsGroup = mRegistry.group<ModelComponent>();
		for (auto ent : modelsGroup)
		{
			auto& [model] = modelsGroup.get<ModelComponent>(ent);
		
			if (model == nullptr|| !model->IsLoaded())
				continue;
		
			model->Draw(commandBuffer);
		}
	}

	void Scene::OnEvent(Shared<Event> event)
	{
	}

	void Scene::Deserialize(DataFile& data)
	{
		LOG_TO_TERMINAL(Logger::Warn, "Keey an eye on EnTT id and UUID id, the might conflict if I'm not able to properly erase EnTT handles");
		
		size_t entityCount = data["Entities"].GetChildrenCount();

		for (size_t i = 0; i < entityCount; i++)
		{
			// get entity id
			DataFile entityData = data["Entities"][i];
			std::string id = entityData["Id"].GetString();

			// create blank entity
			entt::entity entt = mRegistry.create();
			Entity entity = { shared_from_this(), entt, id };
			
			// add entity name
			std::string name = entityData["Name"].GetString();
			entity.AddComponent<NameComponent>(name);

			// check if transform exists
			if (entityData.Exists("Transform"))
			{
				entity.AddComponent<TransformComponent>();
				auto& component = entity.GetComponent<TransformComponent>();

				// read translation
				auto& dataT = entityData["Transform"]["Translation"];
				component.translation = { dataT["X"].GetDouble(), dataT["Y"].GetDouble(), dataT["Z"].GetDouble() };

				// read rotation
				auto& dataR = entityData["Transform"]["Rotation"];
				component.rotation = { dataR["X"].GetDouble(), dataR["Y"].GetDouble(), dataR["Z"].GetDouble() };
				
				// read scale
				auto& dataS = entityData["Transform"]["Scale"];
				component.scale = { dataS["X"].GetDouble(), dataS["Y"].GetDouble(), dataS["Z"].GetDouble() };
			}

			// check if model exists
			if (entityData.Exists("Model"))
			{
				entity.AddComponent<ModelComponent>();
				auto& component = entity.GetComponent<ModelComponent>();

				component.model = std::make_shared<Model>(mRenderer, Application::GetInstance()->GetCamera());

				component.model->LoadFromFile(entityData["Model"]["Path"].GetString());
				component.model->LoadAlbedoTexture(entityData["Model"]["Albedo"].GetString());
			}

			// check if sound source exists
			if (entityData.Exists("SoundSource"))
			{
				entity.AddComponent<SoundSourceComponent>();
				auto& component = entity.GetComponent<SoundSourceComponent>();

				component.source = std::make_shared<sound::Source>();
				component.source->Create(entityData["SoundSource"]["Path"].GetString());
			}

			// assign the entity to the map
			mEntityMap[id] = entity;
		}
	}

	DataFile Scene::Serialize()
	{
		DataFile save;

		for (auto& entid : mEntityMap)
		{
			Entity* entity = &entid.second;

			// don't considerate nullptr
			if (entity == nullptr)
				continue;

			// discard entities that doesnt have name
			if( !entity->HasComponent<NameComponent>())
				continue;

			// write name and id components, they should exists by default
			std::string uuidComponent = std::to_string(entity->GetUUID());
			std::string nameComponent = entity->GetComponent<NameComponent>().name;

			save["Entities"][uuidComponent]["Id"].SetString(uuidComponent);
			save["Entities"][uuidComponent]["Name"].SetString(nameComponent);

			// write the transform component if it exists
			if (entity->HasComponent<TransformComponent>())
			{
				auto& component = entity->GetComponent<TransformComponent>();
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
			if (entity->HasComponent<ModelComponent>())
			{
				auto& component = entity->GetComponent<ModelComponent>();
				auto& place = save["Entities"][uuidComponent]["Model"];

				place["Path"].SetString(component.model->GetPath());
				place["Albedo"].SetString(component.model->GetAlbedoPath());
			}

			// write sound source component if it exists
			if (entity->HasComponent<SoundSourceComponent>())
			{
				auto& component = entity->GetComponent<SoundSourceComponent>();
				auto& place = save["Entities"][uuidComponent]["SoundSource"];

				place["Path"].SetString(component.source->GetPath());
			}
		}

		return save;
	}
}