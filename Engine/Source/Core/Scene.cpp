#include "epch.h"
#include "Scene.h"

#include "Entity/Entity.h"
#include "Entity/Components/Base.h"
#include "Entity/Components/Renderable.h"
#include "Entity/Components/Scriptable.h"
#include "Entity/Components/Sound.h"

#include "Renderer/Vulkan/VKCommander.h"
#include "UI/GUI.h"

#include <iostream>

namespace Cosmos
{
	Scene::Scene(Shared<Renderer> renderer, Shared<Camera> camera)
		: mRenderer(renderer), mCamera(camera)
	{
		Logger() << "Creating Scene";

		mEntityMap = {};
	}

	Scene::~Scene()
	{
		for (auto& ent : mEntityMap)
		{
			if (mEntityMap.size() == 0)
				break;

			DestroyEntity(&ent.second, false);
		}

		mEntityMap.clear();
		mRegistry.clear();
	}

	void Scene::OnUpdate(float timestep)
	{
		// update skyboxes
		auto skyboxsView = mRegistry.view<TransformComponent, SkyboxComponent>();
		for (auto ent : skyboxsView)
		{
			auto [transformComponent, skyboxComponent] = skyboxsView.get<TransformComponent, SkyboxComponent>(ent);

			if(skyboxComponent.skybox == nullptr)
				continue;
			
			skyboxComponent.skybox->OnUpdate(timestep, transformComponent.GetTransform());
		}

		// update models
		auto modelsView = mRegistry.view<TransformComponent, ModelComponent>();
		for (auto ent : modelsView)
		{
			auto [transformComponent, modelComponent] = modelsView.get<TransformComponent, ModelComponent>(ent);

			if (modelComponent.model == nullptr || !modelComponent.model->IsLoaded())
				continue;

			modelComponent.model->OnUpdate(timestep, transformComponent.GetTransform());
		}

		// update quads
		auto quadsView = mRegistry.view<TransformComponent, QuadComponent>();
		for(auto ent : quadsView)
		{
			auto [transformComponent, quadComponent] = quadsView.get<TransformComponent, QuadComponent>(ent);
		
			if(quadComponent.quad == nullptr)
				continue;
		
			quadComponent.quad->OnUpdate(timestep, transformComponent.GetTransform());
		}
	}

	void Scene::OnRender()
	{
		Application::GetInstance()->GetGUI()->OnRender();

		uint32_t currentFrame = mRenderer->GetCurrentFrame();
		VkDeviceSize offsets[] = { 0 };
		VkCommandBuffer commandBuffer = VKCommander::GetInstance()->GetMainRef()->commandBuffers[currentFrame];

		// draw skyboxes
		auto skyboxView = mRegistry.view<SkyboxComponent>();
		for (auto ent : skyboxView)
		{
			auto& [skybox] = skyboxView.get<SkyboxComponent>(ent);

			if (skybox == nullptr)
				continue;

			skybox->OnRender(commandBuffer);
		}

		// draw models
		auto modelsView = mRegistry.view<ModelComponent>();
		for (auto ent : modelsView)
		{
			auto& [model] = modelsView.get<ModelComponent>(ent);

			if (model == nullptr || !model->IsLoaded())
				continue;

			model->OnRender(commandBuffer);
		}

		// draw quads
		auto quadsView = mRegistry.view<QuadComponent>();
		for(auto ent : quadsView)
		{
			auto& [quad] = quadsView.get<QuadComponent>(ent);
		
			if(quad == nullptr)
				continue;
		
			quad->OnRender(commandBuffer);
		}
	}

	void Scene::OnEvent(Shared<Event> event)
	{
	}

	Entity* Scene::CreateEntity(const char* name)
	{
		UUID id = UUID();
		entt::entity entt = mRegistry.create();

		Entity entity{ this, mRegistry.create(), id };
		entity.AddComponent<NameComponent>(name);
		
		mEntityMap[id] = entity;
		return &mEntityMap[id];
	}

	Entity* Scene::DuplicateEntity(Entity* entity)
	{
		// name is an existing component
		std::string name = entity->GetComponent<NameComponent>().name;
		name.append(" copy");

		Entity* newEnt = CreateEntity(name.c_str());

		// transform
		if (entity->HasComponent<TransformComponent>())
		{
			newEnt->AddComponent<TransformComponent>();
			newEnt->GetComponent<TransformComponent>().rotation = entity->GetComponent<TransformComponent>().rotation;
			newEnt->GetComponent<TransformComponent>().translation = entity->GetComponent<TransformComponent>().translation;
			newEnt->GetComponent<TransformComponent>().scale = entity->GetComponent<TransformComponent>().scale;
		}
		
		// model
		if (entity->HasComponent<ModelComponent>())
		{
			newEnt->AddComponent<ModelComponent>();
			newEnt->GetComponent<ModelComponent>().model = CreateShared<Model>(mRenderer, mCamera);

			// testing
			auto previousModel = entity->GetComponent<ModelComponent>().model;

			if (previousModel->IsLoaded())
			{
				newEnt->GetComponent<ModelComponent>().model->LoadFromFile(previousModel->GetPath());

				if (previousModel->IsCustomAlbedoLoaded())
				{
					newEnt->GetComponent<ModelComponent>().model->LoadAlbedoTexture(previousModel->GetAlbedoPath());
				}
			}
		}

		return newEnt;
	}

	void Scene::DestroyEntity(Entity* entity, bool eraseFromEntitymap)
	{
		if (entity == nullptr)
			return;

		if (entity->HasComponent<ModelComponent>())
		{
			if (entity->GetComponent<ModelComponent>().model != nullptr)
			{
				entity->GetComponent<ModelComponent>().model->Destroy();
			}
		}

		if (entity->HasComponent<SkyboxComponent>())
		{
			if(entity->GetComponent<SkyboxComponent>().skybox != nullptr)
			{
				entity->GetComponent<SkyboxComponent>().skybox->Destroy();
			}
		}

		if(eraseFromEntitymap)
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
			Entity entity = { this, entt, id };
			
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