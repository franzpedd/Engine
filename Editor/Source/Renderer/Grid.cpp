#include "Grid.h"

#include "UI/Viewport.h"

#include <chrono>

namespace Cosmos
{
	Grid::Grid(std::shared_ptr<Renderer>& renderer, Scene* scene)
		: Widget("Grid"), mScene(scene), mRenderer(renderer)
	{
		Logger() << "Creating Grid";

		CreateResources();
	}

	void Grid::OnRenderDraw()
	{
		if (!mVisible) return;

		uint32_t currentFrame = mRenderer->CurrentFrame();
		VkDeviceSize offsets[] = { 0 };
		VkCommandBuffer cmdBuffer = Commander::Get().GetPrimary()->commandBuffers[currentFrame];

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[currentFrame], 0, nullptr);
		vkCmdDraw(cmdBuffer, 6, 1, 0, 0);
	}

	void Grid::OnUpdate()
	{
		UniformBufferObject ubo = {};
		ubo.model = glm::mat4(1.0f);
		ubo.view = mScene->GetCamera()->GetView();
		ubo.proj = mScene->GetCamera()->GetProjection();
		
		memcpy(mUniformBuffersMapped[mRenderer->CurrentFrame()], &ubo, sizeof(ubo));
	}

	void Grid::OnDestroy()
	{
		vkDeviceWaitIdle(mRenderer->GetDevice()->GetDevice());

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(mRenderer->GetDevice()->GetDevice(), mUniformBuffers[i], nullptr);
			vkFreeMemory(mRenderer->GetDevice()->GetDevice(), mUniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(mRenderer->GetDevice()->GetDevice(), mDescriptorPool, nullptr);
		vkDestroyDescriptorSetLayout(mRenderer->GetDevice()->GetDevice(), mDescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(mRenderer->GetDevice()->GetDevice(), mPipelineLayout, nullptr);
		vkDestroyPipeline(mRenderer->GetDevice()->GetDevice(), mGraphicsPipeline, nullptr);
	}

	void Grid::ToogleOnOff()
	{
		mVisible == true ? mVisible = false : mVisible = true;
	}

	void Grid::CreateResources()
	{
		// create graphics pipeline related (shaders, descriptor set layout, pipeline layout, graphics pipeline)
		{
			mVertexShader = VKShader::Create(std::dynamic_pointer_cast<VKDevice>(mRenderer->GetDevice()), VKShader::Vertex, "Grid.vert", util::GetAssetSubDir("Shaders/grid.vert"));
			mFragmentShader = VKShader::Create(std::dynamic_pointer_cast<VKDevice>(mRenderer->GetDevice()), VKShader::Fragment, "Grid.frag", util::GetAssetSubDir("Shaders/grid.frag"));

			const std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			const std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { mVertexShader->Stage(), mFragmentShader->Stage() };

			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
			uboLayoutBinding.pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo descSetLayoutCI = {};
			descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descSetLayoutCI.pNext = nullptr;
			descSetLayoutCI.flags = 0;
			descSetLayoutCI.bindingCount = 1;
			descSetLayoutCI.pBindings = &uboLayoutBinding;
			VK_ASSERT(vkCreateDescriptorSetLayout(mRenderer->GetDevice()->GetDevice(), &descSetLayoutCI, nullptr, &mDescriptorSetLayout), "Failed to create descriptor set layout");

			VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
			pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCI.pNext = nullptr;
			pipelineLayoutCI.flags = 0;
			pipelineLayoutCI.setLayoutCount = 1;
			pipelineLayoutCI.pSetLayouts = &mDescriptorSetLayout;
			VK_ASSERT(vkCreatePipelineLayout(mRenderer->GetDevice()->GetDevice(), &pipelineLayoutCI, nullptr, &mPipelineLayout), "Failed to create descriptor set layout");

			VkPipelineVertexInputStateCreateInfo VISCI = {};
			VISCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			VISCI.pNext = nullptr;
			VISCI.flags = 0;
			VISCI.vertexAttributeDescriptionCount = 0;
			VISCI.pVertexAttributeDescriptions = nullptr;
			VISCI.vertexBindingDescriptionCount = 0;
			VISCI.pVertexBindingDescriptions = nullptr;

			VkPipelineInputAssemblyStateCreateInfo IASCI = {};
			IASCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
			IASCI.pNext = nullptr;
			IASCI.flags = 0;
			IASCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
			IASCI.primitiveRestartEnable = VK_FALSE;

			VkPipelineViewportStateCreateInfo VSCI = {};
			VSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
			VSCI.pNext = nullptr;
			VSCI.flags = 0;
			VSCI.viewportCount = 1;
			VSCI.scissorCount = 1;

			VkPipelineRasterizationStateCreateInfo RSCI = {};
			RSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
			RSCI.pNext = nullptr;
			RSCI.flags = 0;
			RSCI.depthClampEnable = VK_FALSE;
			RSCI.rasterizerDiscardEnable = VK_FALSE;
			RSCI.polygonMode = VK_POLYGON_MODE_FILL;
			RSCI.lineWidth = 1.0f;
			RSCI.cullMode = VK_CULL_MODE_NONE;
			RSCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
			RSCI.depthBiasEnable = VK_FALSE;

			VkPipelineMultisampleStateCreateInfo MSCI = {};
			MSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
			MSCI.pNext = nullptr;
			MSCI.flags = 0;
			MSCI.sampleShadingEnable = VK_FALSE;
			MSCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

			VkPipelineDepthStencilStateCreateInfo DSSCI = {};
			DSSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
			DSSCI.depthTestEnable = VK_TRUE;
			DSSCI.depthWriteEnable = VK_TRUE;
			DSSCI.depthCompareOp = VK_COMPARE_OP_LESS;
			DSSCI.depthBoundsTestEnable = VK_FALSE;
			DSSCI.stencilTestEnable = VK_FALSE;

			VkPipelineColorBlendAttachmentState CBAS = {};
			CBAS.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
			CBAS.blendEnable = VK_FALSE;
			CBAS.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			CBAS.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			CBAS.colorBlendOp = VK_BLEND_OP_ADD;
			CBAS.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
			CBAS.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;	
			CBAS.alphaBlendOp = VK_BLEND_OP_ADD;

			VkPipelineColorBlendStateCreateInfo CBSCI = {};
			CBSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
			CBSCI.pNext = nullptr;
			CBSCI.flags = 0;
			CBSCI.logicOpEnable = VK_FALSE;
			CBSCI.logicOp = VK_LOGIC_OP_COPY;
			CBSCI.attachmentCount = 1;
			CBSCI.pAttachments = &CBAS;
			CBSCI.blendConstants[0] = 0.0f;
			CBSCI.blendConstants[1] = 0.0f;
			CBSCI.blendConstants[2] = 0.0f;
			CBSCI.blendConstants[3] = 0.0f;

			VkPipelineDynamicStateCreateInfo DSCI = {};
			DSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
			DSCI.pNext = nullptr;
			DSCI.flags = 0;
			DSCI.dynamicStateCount = (uint32_t)dynamicStates.size();
			DSCI.pDynamicStates = dynamicStates.data();

			VkGraphicsPipelineCreateInfo pipelineCI = {};
			pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
			pipelineCI.stageCount = 2;
			pipelineCI.pStages = shaderStages.data();
			pipelineCI.pVertexInputState = &VISCI;
			pipelineCI.pInputAssemblyState = &IASCI;
			pipelineCI.pViewportState = &VSCI;
			pipelineCI.pRasterizationState = &RSCI;
			pipelineCI.pMultisampleState = &MSCI;
			pipelineCI.pDepthStencilState = &DSSCI;
			pipelineCI.pColorBlendState = &CBSCI;
			pipelineCI.pDynamicState = &DSCI;
			pipelineCI.layout = mPipelineLayout;
			pipelineCI.renderPass = Commander::Get().GetPrimary()->renderPass;
			pipelineCI.subpass = 0;
			pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateGraphicsPipelines(mRenderer->GetDevice()->GetDevice(), mRenderer->PipelineCache(), 1, &pipelineCI, nullptr, &mGraphicsPipeline), "Failed to create graphics pipeline");
		}

		// matrix ubo
		{
			mUniformBuffers.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			mUniformBuffersMemory.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			mUniformBuffersMapped.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);

			for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
			{
				BufferCreate
				(
					mRenderer->GetDevice(),
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					sizeof(UniformBufferObject),
					&mUniformBuffers[i],
					&mUniformBuffersMemory[i]
				);

				vkMapMemory(mRenderer->GetDevice()->GetDevice(), mUniformBuffersMemory[i], 0, sizeof(UniformBufferObject), 0, &mUniformBuffersMapped[i]);
			}
		}

		// create descriptor pool and descriptor sets
		{
			VkDescriptorPoolSize poolSize = {};
			poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSize.descriptorCount = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;

			VkDescriptorPoolCreateInfo descPoolCI = {};
			descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descPoolCI.poolSizeCount = 1;
			descPoolCI.pPoolSizes = &poolSize;
			descPoolCI.maxSets = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;
			VK_ASSERT(vkCreateDescriptorPool(mRenderer->GetDevice()->GetDevice(), &descPoolCI, nullptr, &mDescriptorPool), "Failed to create descriptor pool");

			std::vector<VkDescriptorSetLayout> layouts(RENDERER_MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayout);
			
			VkDescriptorSetAllocateInfo descSetAllocInfo = {};
			descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descSetAllocInfo.descriptorPool = mDescriptorPool;
			descSetAllocInfo.descriptorSetCount = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;
			descSetAllocInfo.pSetLayouts = layouts.data();

			mDescriptorSets.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			VK_ASSERT(vkAllocateDescriptorSets(mRenderer->GetDevice()->GetDevice(), &descSetAllocInfo, mDescriptorSets.data()), "Failed to allocate descriptor sets");
		
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
				
				vkUpdateDescriptorSets(mRenderer->GetDevice()->GetDevice(), 1, &descriptorWrite, 0, nullptr);
			}
		}

		// destroy the shader module after usage
		vkDestroyShaderModule(mRenderer->GetDevice()->GetDevice(), mVertexShader->Module(), nullptr);
		vkDestroyShaderModule(mRenderer->GetDevice()->GetDevice(), mFragmentShader->Module(), nullptr);
	}
}