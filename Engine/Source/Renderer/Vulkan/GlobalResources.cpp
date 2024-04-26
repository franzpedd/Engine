#include "epch.h"
#include "GlobalResources.h"

#include "VKCommander.h"
#include "VKInitializers.h"
#include "VKShader.h"
#include "VKVertex.h"

#include "Util/FileSystem.h"

namespace Cosmos
{
	void ModelGlobalResource::Initialize(std::shared_ptr<VKDevice> device, VkPipelineCache cache)
	{
		// free used resources if the exists
		{
			if (pipeline != VK_NULL_HANDLE) 
				vkDestroyPipeline(device->GetDevice(), pipeline, nullptr);

			if (pipelineLayout != VK_NULL_HANDLE) 
				vkDestroyPipelineLayout(device->GetDevice(), pipelineLayout, nullptr);

			if (descriptorSetLayout != VK_NULL_HANDLE) 
				vkDestroyDescriptorSetLayout(device->GetDevice(), descriptorSetLayout, nullptr);
		}

		// descriptor set and pipeline layout
		{
			VkDescriptorSetLayoutBinding uboBinding = vulkan::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);
			VkDescriptorSetLayoutBinding albedoBinding = vulkan::DescriptorSetLayoutBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

			const std::vector<VkDescriptorSetLayoutBinding> bindings = { uboBinding, albedoBinding };

			VkDescriptorSetLayoutCreateInfo descSetLayoutCI = vulkan::DescriptorSetLayoutCreateInfo(bindings);
			VK_ASSERT(vkCreateDescriptorSetLayout(device->GetDevice(), &descSetLayoutCI, nullptr, &descriptorSetLayout), "Failed to create descriptor set layout");

			VkPipelineLayoutCreateInfo pipelineLayoutCI = vulkan::PipelineLayouCreateInfo(&descriptorSetLayout);
			VK_ASSERT(vkCreatePipelineLayout(device->GetDevice(), &pipelineLayoutCI, nullptr, &pipelineLayout), "Failed to create pipeline layout");
		}

		// pipeline
		{
			// shaders
			Unique<VKShader> vShader = CreateUnique<VKShader>(device, VKShader::Type::Vertex, "Model.vert", GetAssetSubDir("Shaders/model.vert"));
			Unique<VKShader> fShader = CreateUnique<VKShader>(device, VKShader::Type::Fragment, "Model.frag", GetAssetSubDir("Shaders/model.frag"));

			// constants
			const std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			const std::vector<VkVertexInputAttributeDescription> attributeDesc = VKVertex::GetAttributeDescriptions();
			const std::vector<VkVertexInputBindingDescription> bindingDesc = VKVertex::GetBindingDescription();
			const std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vShader->GetShaderStageCreateInfoRef(), fShader->GetShaderStageCreateInfoRef()};

			// pipeline objects
			VkPipelineVertexInputStateCreateInfo VISCI = vulkan::PipelineVertexInputStateCreateInfo(bindingDesc, attributeDesc);
			VkPipelineInputAssemblyStateCreateInfo IASCI = vulkan::PipelineInputAssemblyStateCrateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
			VkPipelineViewportStateCreateInfo VSCI = vulkan::PipelineViewportStateCreateInfo(1, 1);
			VkPipelineRasterizationStateCreateInfo RSCI = vulkan::PipelineRasterizationCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
			VkPipelineMultisampleStateCreateInfo MSCI = vulkan::PipelineMultisampleStateCreateInfo(VKCommander::GetInstance()->GetMainRef()->msaa);
			VkPipelineDepthStencilStateCreateInfo DSSCI = vulkan::PipelineDepthStencilStateCreateInfo(VK_TRUE, VK_TRUE, VK_COMPARE_OP_LESS);
			VkPipelineColorBlendAttachmentState CBAS = vulkan::PipelineColorBlendAttachmentState(VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT, VK_FALSE);
			VkPipelineColorBlendStateCreateInfo CBSCI = vulkan::PipelineColorBlendStateCreateInfo(1, &CBAS);
			VkPipelineDynamicStateCreateInfo DSCI = vulkan::PipelineDynamicStateCreateInfo(dynamicStates);

			VkGraphicsPipelineCreateInfo pipelineCI = {};
			pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCI.stageCount = (uint32_t)shaderStages.size();
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.pVertexInputState = &VISCI;
			pipelineCI.pInputAssemblyState = &IASCI;
			pipelineCI.pViewportState = &VSCI;
			pipelineCI.pRasterizationState = &RSCI;
			pipelineCI.pMultisampleState = &MSCI;
			pipelineCI.pDepthStencilState = &DSSCI;
			pipelineCI.pColorBlendState = &CBSCI;
			pipelineCI.pDynamicState = &DSCI;
			pipelineCI.layout = pipelineLayout;
			pipelineCI.renderPass = VKCommander::GetInstance()->GetMainRef()->renderPass;
			pipelineCI.subpass = 0;
			VK_ASSERT(vkCreateGraphicsPipelines(device->GetDevice(), cache, 1, &pipelineCI, nullptr, &pipeline), "Failed to create graphics pipeline");
		}
	}
}