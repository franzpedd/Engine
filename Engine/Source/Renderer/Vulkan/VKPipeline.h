#pragma once

#include "VKDefines.h"
#include "VKDevice.h"

#include "VKShader.h"
#include "VKVertex.h"

namespace Cosmos
{
    struct VKPipelineSpecification
    {
        // these must be created before creating the VKPipeline
        VkPipelineCache cache;
        Shared<VKShader> vertexShader;
        Shared<VKShader> fragmentShader;
        std::vector<VKVertex::Component> vertexComponents = {};
        std::vector<VkDescriptorSetLayoutBinding> bindings = {};
        
        // these will be auto generated, but can be previously modified between VKPipeline::VKPipeline and VKPipeline::Build for customization
        std::vector<VkDynamicState> dynamicStates { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
        std::vector<VkPipelineShaderStageCreateInfo> shaderStagesCI = {};
        VkPipelineVertexInputStateCreateInfo VISCI = {};
        VkPipelineInputAssemblyStateCreateInfo IASCI = {};
        VkPipelineViewportStateCreateInfo VSCI = {};
        VkPipelineRasterizationStateCreateInfo RSCI = {};
        VkPipelineMultisampleStateCreateInfo MSCI = {};
        VkPipelineDepthStencilStateCreateInfo DSSCI = {};
        VkPipelineColorBlendAttachmentState CBAS = {};
        VkPipelineColorBlendStateCreateInfo CBSCI = {};
        VkPipelineDynamicStateCreateInfo DSCI = {};
    };

    class VKPipeline
    {
    public:

        // constructor with a previously defined specification
        VKPipeline(Shared<VKDevice> device, VKPipelineSpecification specification);

        // destructor
        ~VKPipeline();

        // returns a reference to the pipeline specification
        inline  VKPipelineSpecification& GetSpecificationRef() { return mSpecification; }

        // returns the descriptor set layout
        inline VkDescriptorSetLayout GetDescriptorSetLayout() const { return mDescriptorSetLayout; }

        // returns the pipeline layout
        inline VkPipelineLayout GetPipelineLayout() const { return mPipelineLayout; }

        // returns the pipeline
        inline VkPipeline GetPipeline() const { return mPipeline; }

    public:

        // creates a pipeline object given previously configured struct VKPiplineSpecification
        void Build();

    private:

        Shared<VKDevice> mDevice;
        VKPipelineSpecification mSpecification;
        VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
        VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;
        VkPipeline mPipeline = VK_NULL_HANDLE;
    };
}