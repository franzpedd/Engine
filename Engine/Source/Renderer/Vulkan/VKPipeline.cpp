#include "epch.h"
#include "VKPipeline.h"

#include "VKCommander.h"

namespace Cosmos
{
   VKPipeline::VKPipeline(Shared<VKDevice> device, VKPipelineSpecification specification)
        : mDevice(device), mSpecification(specification)
    {
        // descriptor set and pipeline layout
        VkDescriptorSetLayoutCreateInfo descSetLayoutCI = {};
        descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descSetLayoutCI.pNext = nullptr;
        descSetLayoutCI.flags = 0;
        descSetLayoutCI.bindingCount = (uint32_t)mSpecification.bindings.size();
        descSetLayoutCI.pBindings = mSpecification.bindings.data();
		VK_ASSERT(vkCreateDescriptorSetLayout(device->GetDevice(), &descSetLayoutCI, nullptr, &mDescriptorSetLayout), "Failed to create descriptor set layout");

        VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
        pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutCI.pNext = nullptr;
        pipelineLayoutCI.flags = 0;
        pipelineLayoutCI.setLayoutCount = 1;
        pipelineLayoutCI.pSetLayouts = &mDescriptorSetLayout;
		VK_ASSERT(vkCreatePipelineLayout(mDevice->GetDevice(), &pipelineLayoutCI, nullptr, &mPipelineLayout), "Failed to create pipeline layout");
		
        // shader stages
        mSpecification.shaderStagesCI = 
        {
            mSpecification.vertexShader->GetShaderStageCreateInfoRef(),
            mSpecification.fragmentShader->GetShaderStageCreateInfoRef()
        };

        // pipeline default configuration
        // vertex input state
        mSpecification.VISCI = VKVertex::GetPipelineVertexInputState(mSpecification.vertexComponents);

        // input assembly state
        mSpecification.IASCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        mSpecification.IASCI.pNext = nullptr;
        mSpecification.IASCI.flags = 0;
        mSpecification.IASCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        mSpecification.IASCI.primitiveRestartEnable = VK_FALSE;
        // viewport state
        mSpecification.VSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        mSpecification.VSCI.pNext = nullptr;
        mSpecification.VSCI.flags = 0;
        mSpecification.VSCI.viewportCount = 1;
        mSpecification.VSCI.pViewports = nullptr; // using dynamic viewport
        mSpecification.VSCI.scissorCount = 1;
        mSpecification.VSCI.pScissors = nullptr; // using dynamic scissor
        // rasterization state
        mSpecification.RSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        mSpecification.RSCI.pNext = nullptr;
        mSpecification.RSCI.flags = 0;
        mSpecification.RSCI.polygonMode = VK_POLYGON_MODE_FILL;
        mSpecification.RSCI.cullMode = VK_CULL_MODE_NONE;
        mSpecification.RSCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        mSpecification.RSCI.depthClampEnable = VK_FALSE;
        mSpecification.RSCI.rasterizerDiscardEnable = VK_FALSE;
        mSpecification.RSCI.lineWidth = 1.0f;
        // multisampling state
        mSpecification.MSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        mSpecification.MSCI.pNext = nullptr;
        mSpecification.MSCI.flags = 0;
        mSpecification.MSCI.rasterizationSamples = VKCommander::GetInstance()->GetMainRef()->msaa;
        mSpecification.MSCI.sampleShadingEnable = VK_FALSE;
        // depth stencil state
        mSpecification.DSSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        mSpecification.DSSCI.pNext = nullptr;
        mSpecification.DSSCI.flags = 0;
        mSpecification.DSSCI.depthTestEnable = VK_TRUE;
        mSpecification.DSSCI.depthWriteEnable = VK_TRUE;
        mSpecification.DSSCI.depthCompareOp = VK_COMPARE_OP_LESS;
        mSpecification.DSSCI.back.compareOp = VK_COMPARE_OP_ALWAYS;
        // color blend
        mSpecification.CBAS.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        mSpecification.CBAS.blendEnable = VK_FALSE;
        mSpecification.CBAS.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        mSpecification.CBAS.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        mSpecification.CBAS.colorBlendOp = VK_BLEND_OP_ADD;
        mSpecification.CBAS.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        mSpecification.CBAS.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        mSpecification.CBAS.alphaBlendOp = VK_BLEND_OP_ADD;
        // color blend state
        mSpecification.CBSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        mSpecification.CBSCI.pNext = nullptr;
        mSpecification.CBSCI.flags = 0;
        mSpecification.CBSCI.attachmentCount = 1;
        mSpecification.CBSCI.pAttachments = &mSpecification.CBAS;
        mSpecification.CBSCI.logicOpEnable = VK_FALSE;
        mSpecification.CBSCI.logicOp = VK_LOGIC_OP_COPY;
        mSpecification.CBSCI.blendConstants[0] = 0.0f;
        mSpecification.CBSCI.blendConstants[1] = 0.0f;
        mSpecification.CBSCI.blendConstants[2] = 0.0f;
        mSpecification.CBSCI.blendConstants[3] = 0.0f;
        // dynamic state
        mSpecification.DSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        mSpecification.DSCI.pNext = nullptr;
        mSpecification.DSCI.flags = 0;
        mSpecification.DSCI.dynamicStateCount = (uint32_t)mSpecification.dynamicStates.size();
        mSpecification.DSCI.pDynamicStates = mSpecification.dynamicStates.data();
    }

    VKPipeline::~VKPipeline()
    {
        vkDeviceWaitIdle(mDevice->GetDevice());

        vkDestroyPipeline(mDevice->GetDevice(), mPipeline, nullptr);
        vkDestroyPipelineLayout(mDevice->GetDevice(), mPipelineLayout, nullptr);
        vkDestroyDescriptorSetLayout(mDevice->GetDevice(), mDescriptorSetLayout, nullptr);
    }

    void VKPipeline::Build()
    {
        // pipeline creation
        VkGraphicsPipelineCreateInfo pipelineCI = {};
        pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineCI.pNext = nullptr;
        pipelineCI.flags = 0;
        pipelineCI.stageCount = (uint32_t)mSpecification.shaderStagesCI.size();
        pipelineCI.pStages = mSpecification.shaderStagesCI.data();
        pipelineCI.pVertexInputState = &mSpecification.VISCI;
        pipelineCI.pInputAssemblyState = &mSpecification.IASCI;
        pipelineCI.pViewportState = &mSpecification.VSCI;
        pipelineCI.pRasterizationState = &mSpecification.RSCI;
        pipelineCI.pMultisampleState = &mSpecification.MSCI;
        pipelineCI.pDepthStencilState = &mSpecification.DSSCI;
        pipelineCI.pColorBlendState = &mSpecification.CBSCI;
        pipelineCI.pDynamicState = &mSpecification.DSCI;
        pipelineCI.layout = mPipelineLayout;
        pipelineCI.renderPass = VKCommander::GetInstance()->GetMainRef()->renderPass;
        pipelineCI.subpass = 0;
        VK_ASSERT(vkCreateGraphicsPipelines(mDevice->GetDevice(), mSpecification.cache, 1, &pipelineCI, nullptr, &mPipeline), "Failed to create graphics pipeline");
    }
}