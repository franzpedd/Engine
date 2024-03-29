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

	TextureSampler::AddressMode TextureSampler::WrapMode(int32_t wrap)
	{
		switch (wrap)
		{
		case -1:
		case 10497: return AddressMode::ADDRESS_MODE_REPEAT;
		case 33071: return AddressMode::ADDRESS_MODE_CLAMP_TO_EDGE;
		case 33648: return AddressMode::ADDRESS_MODE_MIRRORED_REPEAT;
		}

		LOG_TO_TERMINAL(Logger::Error, "Unknown wrap mode %d", wrap);
		return AddressMode::ADDRESS_MODE_REPEAT;
	}

	TextureSampler::Filter TextureSampler::FilterMode(int32_t filter)
	{
		switch (filter)
		{
		case -1:
		case 9728: return Filter::FILTER_NEAREST;
		case 9729: return Filter::FILTER_LINEAR;
		case 9984: return Filter::FILTER_NEAREST;
		case 9985: return Filter::FILTER_NEAREST;
		case 9986: return Filter::FILTER_LINEAR;
		case 9987: return Filter::FILTER_LINEAR;
		}

		LOG_TO_TERMINAL(Logger::Error, "Unknown filter mode %d", filter);
		return Filter::FILTER_NEAREST;
	}
}