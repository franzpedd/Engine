#include "Scene.h"

#include "Platform/Window.h"
#include "Util/Logger.h"

namespace Cosmos
{
	std::shared_ptr<Scene> Scene::Create(std::shared_ptr<Window>& window)
	{
		return std::make_shared<Scene>(window);
	}

	Scene::Scene(std::shared_ptr<Window>& window)
		: mWindow(window)
	{
		Logger() << "Creating Scene";
	}

	Scene::~Scene()
	{

	}

	void Scene::OnUpdate(Timestep ts)
	{
		for (auto& ent : mEntities)
		{
			ent->OnUpdate(ts);
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