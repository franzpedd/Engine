#include "epch.h"
#include "Renderer.h"
#include "Vulkan/VKRenderer.h"

namespace Cosmos
{
	std::shared_ptr<Renderer> Renderer::Create()
	{
		return std::make_shared<VKRenderer>();
	}
}