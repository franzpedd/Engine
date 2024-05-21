#include "epch.h"
#include "VKVertex.h"

namespace Cosmos
{
	std::vector<VkVertexInputBindingDescription> VKVertex::bindingDescriptions = {};
	std::vector<VkVertexInputAttributeDescription> VKVertex::attributeDescriptions = {};

	std::vector<VkVertexInputBindingDescription> VKVertex::GetBindingDescriptions()
	{
        bindingDescriptions.clear();
        bindingDescriptions.resize(1);

        bindingDescriptions[0].binding = 0;
        bindingDescriptions[0].stride = sizeof(VKVertex);
        bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

        return bindingDescriptions;
	}

    VkVertexInputAttributeDescription VKVertex::GetInputAttributeDescription(uint32_t binding, uint32_t location, VKVertex::Component component)
    {
        switch (component)
        {
            case VKVertex::Component::POSITION: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VKVertex, position) });
            case VKVertex::Component::COLOR: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(VKVertex, color) });
            case VKVertex::Component::NORMAL: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32B32_SFLOAT, offsetof(VKVertex, normal) });
            case VKVertex::Component::UV0: return VkVertexInputAttributeDescription({ location, binding, VK_FORMAT_R32G32_SFLOAT, offsetof(VKVertex, uv0) });
            default: return VkVertexInputAttributeDescription({});
        }
    }

    std::vector<VkVertexInputAttributeDescription> VKVertex::GetAttributeDescriptions(const std::vector<VKVertex::Component> components)
    {
        std::vector<VkVertexInputAttributeDescription> result = {};
        constexpr uint32_t binding = 0;
        uint32_t location = 0;
        
        for (auto component : components)
        {
            result.push_back(VKVertex::GetInputAttributeDescription(binding, location, component));
            location++;
        }
        return result;
    }

    VkPipelineVertexInputStateCreateInfo VKVertex::GetPipelineVertexInputState(const std::vector<VKVertex::Component> components)
    {
        bindingDescriptions = GetBindingDescriptions();
        attributeDescriptions = GetAttributeDescriptions(components);

        VkPipelineVertexInputStateCreateInfo VISCI = {};
        VISCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        VISCI.pNext = nullptr;
        VISCI.flags = 0;
        VISCI.vertexBindingDescriptionCount = (uint32_t)bindingDescriptions.size();
        VISCI.pVertexBindingDescriptions = bindingDescriptions.data();
        VISCI.vertexAttributeDescriptionCount = (uint32_t)attributeDescriptions.size();
        VISCI.pVertexAttributeDescriptions = attributeDescriptions.data();

        return VISCI;
    }
}