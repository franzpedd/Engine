#include "epch.h"
#include "Renderer.h"
#include "Vulkan/VKRenderer.h"

namespace Cosmos
{
	Shared<Renderer> Renderer::Create()
	{
		return CreateShared<VKRenderer>();
	}
}