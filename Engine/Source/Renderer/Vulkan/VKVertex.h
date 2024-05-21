#pragma once

#include "VKDefines.h"
#include "Util/Math.h"
#include <array>

namespace Cosmos
{
    struct VKVertex
    {
        enum Component 
        {
            POSITION = 0,
            COLOR,
            NORMAL,
            UV0
        };

        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv0;

        // these variables must be hold in memory for the creation of pre-configured pipelines
        static std::vector<VkVertexInputBindingDescription> bindingDescriptions;
        static std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

    public:

        // checks if current vertex is the same as another
        bool operator==(const VKVertex& other) const
        {
            return position == other.position
                && color == other.color
                && normal == other.normal
                && uv0 == other.uv0;
        }

    public:

        // returns the binding descriptions, currently it is very simple and only has one binding, used internally
        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();

        // returns the attribute description based on configuration, used internally
        static VkVertexInputAttributeDescription GetInputAttributeDescription(uint32_t binding, uint32_t location, VKVertex::Component component);

        // returns the attribute descriptions, it generates the attributes based on a list of desired attributes, used internally
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions(const std::vector<VKVertex::Component> components);

        // returns the pipeline vertex input state based on desired components
        static VkPipelineVertexInputStateCreateInfo GetPipelineVertexInputState(const std::vector<VKVertex::Component> components);
    }; 
}