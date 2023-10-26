#include "VKPipeline.h"

#include "VKBuffer.h"
#include "VKDevice.h"
#include "VKShader.h"
#include "Util/Logger.h"

namespace Cosmos
{
	std::shared_ptr<VKGraphicsPipeline> VKGraphicsPipeline::Create(std::shared_ptr<VKDevice>& device, InitializerList& initializerList)
	{
		return std::make_shared<VKGraphicsPipeline>(device, initializerList);
	}

	VKGraphicsPipeline::VKGraphicsPipeline(std::shared_ptr<VKDevice>& device, InitializerList& initializerList)
		: mDevice(device), mInitializerList(initializerList)
	{
		CreatePipeline();
		CreateUniformBufferObject();
		CreateDescriptors();
	}

	void VKGraphicsPipeline::Destroy()
	{
		vkDeviceWaitIdle(mDevice->Device());

		mInitializerList.vertexShader->Destroy();
		mInitializerList.fragmentShader->Destroy();

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(mDevice->Device(), mUniformBuffers[i], nullptr);
			vkFreeMemory(mDevice->Device(), mUniformBuffersMemory[i], nullptr);
		}
		
		vkDestroyDescriptorPool(mDevice->Device(), mDescriptorPool, nullptr);
		vkDestroyPipeline(mDevice->Device(), mPipeline, nullptr);
		vkDestroyPipelineLayout(mDevice->Device(), mPipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(mDevice->Device(), mDescriptorSetLayout, nullptr);
	}

	void VKGraphicsPipeline::CreatePipeline()
	{
		VkDescriptorSetLayoutBinding uboDescSetLayoutBinding = {};
		uboDescSetLayoutBinding.binding = 0;
		uboDescSetLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboDescSetLayoutBinding.descriptorCount = 1;
		uboDescSetLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT; // grid example passes to fragment, but that is a special case
		uboDescSetLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutCreateInfo uboDescSetLayoutCI = {};
		uboDescSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		uboDescSetLayoutCI.pNext = nullptr;
		uboDescSetLayoutCI.flags = 0;
		uboDescSetLayoutCI.bindingCount = 1;
		uboDescSetLayoutCI.pBindings = &uboDescSetLayoutBinding;
		VK_ASSERT(vkCreateDescriptorSetLayout(mDevice->Device(), &uboDescSetLayoutCI, nullptr, &mDescriptorSetLayout), "Failed to create descriptor set layout");

		// graphics pipeline
		VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.pNext = nullptr;
		pipelineLayoutCI.flags = 0;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &mDescriptorSetLayout;
		VK_ASSERT(vkCreatePipelineLayout(mDevice->Device(), &pipelineLayoutCI, nullptr, &mPipelineLayout), "Failed to create pipeline layout");

		VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {};
		vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCI.pNext = nullptr;
		vertexInputStateCI.flags = 0;
		vertexInputStateCI.vertexBindingDescriptionCount = (uint32_t)mInitializerList.bindings.size();
		vertexInputStateCI.pVertexBindingDescriptions = mInitializerList.bindings.data();
		vertexInputStateCI.vertexAttributeDescriptionCount = (uint32_t)mInitializerList.attributes.size();
		vertexInputStateCI.pVertexAttributeDescriptions = mInitializerList.attributes.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = {};
		inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateCI.pNext = nullptr;
		inputAssemblyStateCI.flags = 0;
		inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;

		VkPipelineViewportStateCreateInfo viewportStateCI = {};
		viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCI.pNext = nullptr;
		viewportStateCI.flags = 0;
		viewportStateCI.viewportCount = 1;
		viewportStateCI.scissorCount = 1;

		VkPipelineRasterizationStateCreateInfo rasterizationStateCI = {};
		rasterizationStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizationStateCI.pNext = nullptr;
		rasterizationStateCI.flags = 0;
		rasterizationStateCI.depthClampEnable = VK_FALSE;
		rasterizationStateCI.rasterizerDiscardEnable = VK_FALSE;
		rasterizationStateCI.polygonMode = VK_POLYGON_MODE_FILL;
		rasterizationStateCI.lineWidth = 1.0f;
		rasterizationStateCI.cullMode = VK_CULL_MODE_NONE;
		rasterizationStateCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		rasterizationStateCI.depthBiasEnable = VK_FALSE;

		LOG_TO_TERMINAL(Logger::Severity::Warn, "Check MSAA for plane creation");
		VkPipelineMultisampleStateCreateInfo multisampleStateCI = {};
		multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCI.pNext = nullptr;
		multisampleStateCI.flags = 0;
		multisampleStateCI.sampleShadingEnable = VK_FALSE;
		multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT; // check MSAA used by device

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = {};
		depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateCI.depthTestEnable = VK_TRUE;
		depthStencilStateCI.depthWriteEnable = VK_TRUE;
		depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateCI.stencilTestEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentState.blendEnable = VK_FALSE;

		VkPipelineColorBlendStateCreateInfo colorBlendStateCI = {};
		colorBlendStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlendStateCI.pNext = nullptr;
		colorBlendStateCI.flags = 0;
		colorBlendStateCI.logicOpEnable = VK_FALSE;
		colorBlendStateCI.logicOp = VK_LOGIC_OP_COPY;
		colorBlendStateCI.attachmentCount = 1;
		colorBlendStateCI.pAttachments = &colorBlendAttachmentState;
		colorBlendStateCI.blendConstants[0] = 0.0f;
		colorBlendStateCI.blendConstants[1] = 0.0f;
		colorBlendStateCI.blendConstants[2] = 0.0f;
		colorBlendStateCI.blendConstants[3] = 0.0f;

		VkPipelineDynamicStateCreateInfo dynamicStateCI = {};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.pNext = nullptr;
		dynamicStateCI.flags = 0;
		dynamicStateCI.dynamicStateCount = (uint32_t)mInitializerList.dynamicStates.size();
		dynamicStateCI.pDynamicStates = mInitializerList.dynamicStates.data();

		const std::vector<VkPipelineShaderStageCreateInfo> shaderStages =
		{
			mInitializerList.vertexShader->Stage(),
			mInitializerList.fragmentShader->Stage()
		};

		VkGraphicsPipelineCreateInfo graphicsPipelineCI = {};
		graphicsPipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		graphicsPipelineCI.stageCount = (uint32_t)shaderStages.size();
		graphicsPipelineCI.pStages = shaderStages.data();
		graphicsPipelineCI.pVertexInputState = &vertexInputStateCI;
		graphicsPipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
		graphicsPipelineCI.pViewportState = &viewportStateCI;
		graphicsPipelineCI.pRasterizationState = &rasterizationStateCI;
		graphicsPipelineCI.pMultisampleState = &multisampleStateCI;
		graphicsPipelineCI.pDepthStencilState = &depthStencilStateCI;
		graphicsPipelineCI.pColorBlendState = &colorBlendStateCI;
		graphicsPipelineCI.pDynamicState = &dynamicStateCI;
		graphicsPipelineCI.layout = mPipelineLayout;
		graphicsPipelineCI.renderPass = mInitializerList.renderPass;
		graphicsPipelineCI.subpass = 0;
		graphicsPipelineCI.basePipelineHandle = VK_NULL_HANDLE;
		VK_ASSERT(vkCreateGraphicsPipelines(mDevice->Device(), mInitializerList.pipelineCache, 1, &graphicsPipelineCI, nullptr, &mPipeline), "Failed to create graphics pipeline");
	}

	void VKGraphicsPipeline::CreateUniformBufferObject()
	{
		mUniformBuffers.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
		mUniformBuffersMemory.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
		mUniformBuffersMapped.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			BufferCreate
			(
				mDevice,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				sizeof(UniformBufferObject),
				&mUniformBuffers[i],
				&mUniformBuffersMemory[i]
			);

			vkMapMemory(mDevice->Device(), mUniformBuffersMemory[i], 0, sizeof(UniformBufferObject), 0, &mUniformBuffersMapped[i]);
		}
	}

	void VKGraphicsPipeline::CreateDescriptors()
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;

		VkDescriptorPoolCreateInfo descPoolCI = {};
		descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descPoolCI.poolSizeCount = 1;
		descPoolCI.pPoolSizes = &poolSize;
		descPoolCI.maxSets = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;
		VK_ASSERT(vkCreateDescriptorPool(mDevice->Device(), &descPoolCI, nullptr, &mDescriptorPool), "Failed to create descriptor pool");

		std::vector<VkDescriptorSetLayout> layouts(RENDERER_MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayout);

		VkDescriptorSetAllocateInfo descSetAllocInfo = {};
		descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descSetAllocInfo.descriptorPool = mDescriptorPool;
		descSetAllocInfo.descriptorSetCount = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;
		descSetAllocInfo.pSetLayouts = layouts.data();

		mDescriptorSets.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
		VK_ASSERT(vkAllocateDescriptorSets(mDevice->Device(), &descSetAllocInfo, mDescriptorSets.data()), "Failed to allocate descriptor sets");

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = mUniformBuffers[i];
			bufferInfo.offset = 0;
			bufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = mDescriptorSets[i];
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;

			vkUpdateDescriptorSets(mDevice->Device(), 1, &descriptorWrite, 0, nullptr);
		}
	}

	void VKPipelineLibrary::DestroyAllPipelines()
	{
		for (auto& pipeline : mGraphicsTable)
		{
			pipeline.second->Destroy();
		}
	}
}

