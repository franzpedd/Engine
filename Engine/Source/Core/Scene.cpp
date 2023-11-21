#include "Scene.h"

#include "Entity/Entity.h"
#include "Entity/Camera.h"
#include "Platform/Window.h"
#include "Util/Logger.h"
#include "Debug/Profiler.h"

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

		for (auto& ent : mEntities->GetEntitiesVector())
		{
			ent->OnUpdate(timestep);
		}
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
}