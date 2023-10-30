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

	void Scene::OnUpdate(float timestep)
	{
		for (auto& ent : mEntities)
		{
			ent->OnUpdate(timestep);
		}
	}

	void Scene::OnRenderDraw()
	{
		for (auto& ent : mEntities)
		{
			ent->OnRenderDraw();
		}
	}

	void Scene::Destroy()
	{
		for (auto& ent : mEntities)
		{
			ent->OnDestroy();
		}
	}
}