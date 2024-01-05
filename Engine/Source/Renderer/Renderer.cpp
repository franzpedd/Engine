#include "epch.h"
#include "Renderer.h"
#include "Vulkan/VKRenderer.h"

namespace Cosmos
{
	std::shared_ptr<Renderer> Renderer::Create(std::shared_ptr<Window>& window, Scene* scene)
	{
		return std::make_shared<VKRenderer>(window, scene);
	}
}