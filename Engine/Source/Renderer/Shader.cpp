#include "epch.h"
#include "Shader.h"

#include "Device.h"
#include "Vulkan/VKDevice.h"
#include "Vulkan/VKShader.h"

namespace Cosmos
{
	std::shared_ptr<Shader> Shader::Create(std::shared_ptr<Device> device, Type type, std::string name, std::string path)
	{
		return std::make_shared<VKShader>(std::dynamic_pointer_cast<VKDevice>(device), type, name, path);
	}
}