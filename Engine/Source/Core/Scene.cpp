#include "Scene.h"

#include "Util/Logger.h"

namespace Cosmos
{
	std::shared_ptr<Scene> Scene::Create()
	{
		return std::make_shared<Scene>();
	}

	Scene::Scene()
	{
		Logger() << "Creating Scene";
	}

	Scene::~Scene()
	{

	}

	void Scene::OnUpdate()
	{
		for (auto& ent : mEntities)
		{
			ent->OnUpdate();
		}
	}

	void Scene::OnDraw()
	{
		for (auto& ent : mEntities)
		{
			ent->OnDraw();
		}
	}

	void Scene::Destroy()
	{
		for (auto& ent : mEntities)
		{
			ent->OnDrestroy();
		}
	}
}