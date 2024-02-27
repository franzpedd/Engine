#include "Grid.h"

#include "UI/Viewport.h"

#include <chrono>

namespace Cosmos
{
	Grid::Grid(std::shared_ptr<Renderer>& renderer)
		: Widget("Grid"), mRenderer(renderer)
	{
		Logger() << "Creating Grid";

		CreateResources();
	}

	Grid::~Grid()
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
		ubo.view = Scene::Get()->GetCamera()->GetView();
		ubo.proj = Scene::Get()->GetCamera()->GetProjection();
		
		memcpy(mUniformBuffersMapped[mRenderer->CurrentFrame()], &ubo, sizeof(ubo));
	}

	void Grid::ToogleOnOff()
	{
		mVisible == true ? mVisible = false : mVisible = true;
	}

	void Grid::CreateResources()
	{
		// descriptor set and layout bindings
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings =
			{
				vulkan::DescriptorSetLayoutBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
			};

			VkDescriptorSetLayoutCreateInfo descriptorSetCI = vulkan::DescriptorSetLayoutCreateInfo(bindings);
			VK_ASSERT(vkCreateDescriptorSetLayout(mRenderer->GetDevice()->GetDevice(), &descriptorSetCI, nullptr, &mDescriptorSetLayout), "Failed to create descriptor set layout");
			
			VkPipelineLayoutCreateInfo pipelineLayoutCI = vulkan::PipelineLayouCreateInfo(&mDescriptorSetLayout);
			VK_ASSERT(vkCreatePipelineLayout(mRenderer->GetDevice()->GetDevice(), &pipelineLayoutCI, nullptr, &mPipelineLayout), "Failed to create descriptor set layout");
		}

		// create pipeline
		{
			// shaders
			std::shared_ptr<Shader> vShader = Shader::Create(mRenderer->GetDevice(), Shader::Vertex, "Grid.vert", util::GetAssetSubDir("Shaders/grid.vert"));
			std::shared_ptr<Shader> fShader = Shader::Create(mRenderer->GetDevice(), Shader::Fragment, "Grid.frag", util::GetAssetSubDir("Shaders/grid.frag"));

			// constants
			const std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			const std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vShader->GetStage(), fShader->GetStage() };
			const std::vector<VkVertexInputBindingDescription> bindings = {}; // emtpy
			const std::vector<VkVertexInputAttributeDescription> attributes = {}; // emtpy
			
			// pipeline objects
			VkPipelineVertexInputStateCreateInfo VISCI = vulkan::PipelineVertexInputStateCreateInfo(bindings, attributes);
			VkPipelineInputAssemblyStateCreateInfo IASCI = vulkan::PipelineInputAssemblyStateCrateInfo(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST, 0, VK_FALSE);
			VkPipelineViewportStateCreateInfo VSCI = vulkan::PipelineViewportStateCreateInfo(1, 1);
			VkPipelineRasterizationStateCreateInfo RSCI = vulkan::PipelineRasterizationCreateInfo(VK_POLYGON_MODE_FILL, VK_CULL_MODE_NONE, VK_FRONT_FACE_COUNTER_CLOCKWISE);
			VkPipelineMultisampleStateCreateInfo MSCI = vulkan::PipelineMultisampleStateCreateInfo(Commander::Get().GetPrimary()->msaa);
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
			pipelineCI.layout = mPipelineLayout;
			pipelineCI.renderPass = Commander::Get().GetPrimary()->renderPass;
			pipelineCI.subpass = 0;
			VK_ASSERT(vkCreateGraphicsPipelines(mRenderer->GetDevice()->GetDevice(), mRenderer->PipelineCache(), 1, &pipelineCI, nullptr, &mGraphicsPipeline), "Failed to create graphics pipeline");

			// free resources
			vkDestroyShaderModule(mRenderer->GetDevice()->GetDevice(), vShader->GetModule(), nullptr);
			vkDestroyShaderModule(mRenderer->GetDevice()->GetDevice(), fShader->GetModule(), nullptr);
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
	}
}