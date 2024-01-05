#include "epch.h"
#include "Texture.h"

#include "Vulkan/VKDevice.h"
#include "Vulkan/VKTexture.h"

namespace Cosmos
{
	std::shared_ptr<Texture2D> Cosmos::Texture2D::Create(std::shared_ptr<Device> device, const char* path, MSAA msaa, bool ktx)
	{
		return std::make_shared<VKTexture2D>(std::dynamic_pointer_cast<VKDevice>(device), path, (VkSampleCountFlagBits)msaa, ktx);
	}
}