#include "epch.h"
#include "Model.h"

#include "Entity/Camera.h"
#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VKBuffer.h"
#include "Renderer/Vulkan/VKShader.h"

namespace Cosmos
{
	std::shared_ptr<Model> Model::Create(std::shared_ptr<Renderer>& renderer, Camera& camera)
	{
		return std::make_shared<Model>(renderer, camera);
	}

	Model::Model(std::shared_ptr<Renderer>& renderer, Camera& camera)
		: mRenderer(renderer), mCamera(camera)
	{

	}

	Model::~Model()
	{

	}

	void Model::Update(float deltaTime)
	{
		UniformBufferObject ubo = {};
		ubo.model = glm::mat4(1.0f);
		ubo.view = mCamera.GetView();
		ubo.proj = mCamera.GetProjection();
		
		memcpy(mUniformBuffersMapped[mRenderer->CurrentFrame()], &ubo, sizeof(ubo));
	}

	void Model::Draw(VkCommandBuffer commandBuffer)
	{
		VkDeviceSize offsets[1] = { 0 };
		uint32_t currentFrame = mRenderer->CurrentFrame();

		vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[currentFrame], 0, nullptr);
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mMesh->GetVertexBuffer(), offsets);
		vkCmdBindIndexBuffer(commandBuffer, mMesh->GetIndexBuffer(), 0, VK_INDEX_TYPE_UINT32);

		vkCmdDrawIndexed(commandBuffer, mMesh->GetIndexCount(), 1, 0, 0, 0);
	}

	void Model::LoadFromFile(std::string path)
	{
		std::vector<MeshLoader::VertexLayout> vertexLayout =
		{
			MeshLoader::VertexLayout::VERTEX_LAYOUT_POSITION,
			MeshLoader::VertexLayout::VERTEX_LAYOUT_NORMAL,
			MeshLoader::VertexLayout::VERTEX_LAYOUT_UV0,
			MeshLoader::VertexLayout::VERTEX_LAYOUT_COLOR
		};

		mMesh = std::make_unique<MeshLoader>(mRenderer);
		mMesh->LoadMesh(path);
		mMesh->CreateRendererResources(vertexLayout, 1.0f);

		CreateResources();

		mPath = path;
		mLoaded = true;
	}

	void Model::Destroy()
	{
		vkDeviceWaitIdle(mRenderer->GetDevice()->GetDevice());

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(mRenderer->GetDevice()->GetDevice(), mUniformBuffers[i], nullptr);
			vkFreeMemory(mRenderer->GetDevice()->GetDevice(), mUniformBuffersMemory[i], nullptr);
		
		}

		vkDestroyPipeline(mRenderer->GetDevice()->GetDevice(), mGraphicsPipeline, nullptr);
		vkDestroyPipelineLayout(mRenderer->GetDevice()->GetDevice(), mPipelineLayout, nullptr);
		vkDestroyDescriptorSetLayout(mRenderer->GetDevice()->GetDevice(), mDescriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(mRenderer->GetDevice()->GetDevice(), mDescriptorPool, nullptr);

		mMesh->DestroyResources();
	}

	void Model::CreateResources()
	{
		// create shaders
		std::shared_ptr<VKShader> vShader = VKShader::Create(std::reinterpret_pointer_cast<VKDevice>(mRenderer->GetDevice()), VKShader::Vertex, "Model.vert", "Data/Shaders/model.vert");
		std::shared_ptr<VKShader> fShader = VKShader::Create(std::reinterpret_pointer_cast<VKDevice>(mRenderer->GetDevice()), VKShader::Fragment, "Model.frag", "Data/Shaders/model.frag");
		const std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vShader->Stage(), fShader->Stage() };

		// combine descriptor and attributes
		VkPipelineVertexInputStateCreateInfo vertexInputStateCI = {};
		vertexInputStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputStateCI.pNext = nullptr;
		vertexInputStateCI.flags = 0;
		vertexInputStateCI.vertexBindingDescriptionCount = (uint32_t)Vertex::GetBindingDescription().size();
		vertexInputStateCI.pVertexBindingDescriptions = Vertex::GetBindingDescription().data();
		vertexInputStateCI.vertexAttributeDescriptionCount = (uint32_t)Vertex::GetBindingAttributes().size();
		vertexInputStateCI.pVertexAttributeDescriptions = Vertex::GetBindingAttributes().data();

		// create descriptor set
		VkDescriptorSetLayoutBinding uboLayoutBinding = {};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		std::vector<VkDescriptorSetLayoutBinding> bindings = { uboLayoutBinding };

		VkDescriptorSetLayoutCreateInfo descSetLayoutCI = {};
		descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descSetLayoutCI.pNext = nullptr;
		descSetLayoutCI.flags = 0;
		descSetLayoutCI.bindingCount = 1;
		descSetLayoutCI.pBindings = &uboLayoutBinding;
		VK_ASSERT(vkCreateDescriptorSetLayout(mRenderer->GetDevice()->GetDevice(), &descSetLayoutCI, nullptr, &mDescriptorSetLayout), "Failed to create descriptor set layout");

		// create pipeline layout
		VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
		pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCI.pNext = nullptr;
		pipelineLayoutCI.flags = 0;
		pipelineLayoutCI.pPushConstantRanges = nullptr;
		pipelineLayoutCI.pushConstantRangeCount = 0;
		pipelineLayoutCI.setLayoutCount = 1;
		pipelineLayoutCI.pSetLayouts = &mDescriptorSetLayout;
		VK_ASSERT(vkCreatePipelineLayout(mRenderer->GetDevice()->GetDevice(), &pipelineLayoutCI, nullptr, &mPipelineLayout), "Failed to create pipeline layout");
		
		// create graphics pipeline
		VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCI = {};
		inputAssemblyStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssemblyStateCI.pNext = nullptr;
		inputAssemblyStateCI.flags = 0;
		inputAssemblyStateCI.primitiveRestartEnable = VK_FALSE;
		inputAssemblyStateCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;

		VkPipelineViewportStateCreateInfo viewportStateCI = {};
		viewportStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportStateCI.pNext = nullptr;
		viewportStateCI.flags = 0;
		viewportStateCI.scissorCount = 1;
		viewportStateCI.viewportCount = 1;

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

		VkPipelineMultisampleStateCreateInfo multisampleStateCI = {};
		multisampleStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampleStateCI.pNext = nullptr;
		multisampleStateCI.flags = 0;
		multisampleStateCI.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

		VkPipelineDepthStencilStateCreateInfo depthStencilStateCI = {};
		depthStencilStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencilStateCI.pNext = nullptr;
		depthStencilStateCI.flags = 0;
		depthStencilStateCI.depthTestEnable = VK_TRUE;
		depthStencilStateCI.depthWriteEnable = VK_TRUE;
		depthStencilStateCI.depthCompareOp = VK_COMPARE_OP_LESS;
		depthStencilStateCI.depthBoundsTestEnable = VK_FALSE;
		depthStencilStateCI.stencilTestEnable = VK_FALSE;
		depthStencilStateCI.front = depthStencilStateCI.back;
		depthStencilStateCI.back.compareOp = VK_COMPARE_OP_ALWAYS;

		VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {};
		colorBlendAttachmentState.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorBlendAttachmentState.blendEnable = VK_FALSE;
		colorBlendAttachmentState.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachmentState.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachmentState.colorBlendOp = VK_BLEND_OP_ADD;
		colorBlendAttachmentState.srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		colorBlendAttachmentState.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorBlendAttachmentState.alphaBlendOp = VK_BLEND_OP_ADD;

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

		const std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };

		VkPipelineDynamicStateCreateInfo dynamicStateCI = {};
		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.pNext = nullptr;
		dynamicStateCI.flags = 0;
		dynamicStateCI.dynamicStateCount = (uint32_t)dynamicStates.size();
		dynamicStateCI.pDynamicStates = dynamicStates.data();

		VkGraphicsPipelineCreateInfo pipelineCI = {};
		pipelineCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCI.pNext = nullptr;
		pipelineCI.flags = 0;
		pipelineCI.renderPass = mRenderer->GetCommander().AccessMainCommandEntry()->renderPass;
		pipelineCI.layout = mPipelineLayout;
		pipelineCI.pVertexInputState = &vertexInputStateCI;
		pipelineCI.pInputAssemblyState = &inputAssemblyStateCI;
		pipelineCI.pRasterizationState = &rasterizationStateCI;
		pipelineCI.pColorBlendState = &colorBlendStateCI;
		pipelineCI.pMultisampleState = &multisampleStateCI;
		pipelineCI.pViewportState = &viewportStateCI;
		pipelineCI.pDepthStencilState = &depthStencilStateCI;
		pipelineCI.pDynamicState = &dynamicStateCI;
		pipelineCI.stageCount = (uint32_t)shaderStages.size();
		pipelineCI.pStages = shaderStages.data();

		VK_ASSERT(vkCreateGraphicsPipelines(mRenderer->GetDevice()->GetDevice(), mRenderer->PipelineCache(), 1, &pipelineCI, nullptr, &mGraphicsPipeline), "Could not create Graphics Pipeline");

		// prepare uniform buffer
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

		// create descriptor pool
		std::array<VkDescriptorPoolSize, 1> poolSizes{};
		poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSizes[0].descriptorCount = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;

		VkDescriptorPoolCreateInfo descriptorPoolCI = {};
		descriptorPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descriptorPoolCI.poolSizeCount = (uint32_t)poolSizes.size();
		descriptorPoolCI.pPoolSizes = poolSizes.data();
		descriptorPoolCI.maxSets = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;
		VK_ASSERT(vkCreateDescriptorPool(mRenderer->GetDevice()->GetDevice(), &descriptorPoolCI, nullptr, &mDescriptorPool), "Failed to create descriptor set");

		// create descriptor set
		std::vector<VkDescriptorSetLayout> layouts(RENDERER_MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayout);
		VkDescriptorSetAllocateInfo descriptorSetAlloc = {};
		descriptorSetAlloc.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descriptorSetAlloc.pNext = nullptr;
		descriptorSetAlloc.descriptorPool = mDescriptorPool;
		descriptorSetAlloc.descriptorSetCount = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;
		descriptorSetAlloc.pSetLayouts = layouts.data();
		mDescriptorSets.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
		VK_ASSERT(vkAllocateDescriptorSets(mRenderer->GetDevice()->GetDevice(), &descriptorSetAlloc, mDescriptorSets.data()), "Failed to allocate descriptor sets");

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo uboInfo = {};
			uboInfo.buffer = mUniformBuffers[i];
			uboInfo.offset = 0;
			uboInfo.range = sizeof(UniformBufferObject);
			
			std::array<VkWriteDescriptorSet, 1> descriptorWrites = {};
			descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrites[0].dstSet = mDescriptorSets[i];
			descriptorWrites[0].dstBinding = 0;
			descriptorWrites[0].dstArrayElement = 0;
			descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrites[0].descriptorCount = 1;
			descriptorWrites[0].pBufferInfo = &uboInfo;
			
			vkUpdateDescriptorSets(mRenderer->GetDevice()->GetDevice(), (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
		}

		vkDestroyShaderModule(mRenderer->GetDevice()->GetDevice(), vShader->Module(), nullptr);
		vkDestroyShaderModule(mRenderer->GetDevice()->GetDevice(), fShader->Module(), nullptr);
	}
}