#include "Scene.h"

#include "Renderer/Renderer.h"
#include "Util/Logger.h"

namespace Cosmos
{
	std::shared_ptr<Scene> Scene::Create(std::shared_ptr<Renderer>& renderer)
	{
		return std::make_shared<Scene>(renderer);
	}

	Scene::Scene(std::shared_ptr<Renderer>& renderer)
		: mRenderer(renderer)
	{
		Logger() << "Creating Scene";
	}
	
	Scene::~Scene()
	{

	}
}