#include "VKDrawable.h"

#include "VKDevice.h"
#include "VKShader.h"
#include "VKUtility.h"
#include "Util/Logger.h"

namespace Cosmos
{
	VKDrawable::VKDrawable(std::shared_ptr<VKDevice>& device)
		: mDevice(device)
	{
		Logger() << "Creating VKDrawable";
	}

	VKDrawable::~VKDrawable()
	{

	}

	VkVertexInputBindingDescription UBO::Vertex::BindingDescription()
	{
		VkVertexInputBindingDescription bindings = {};
		bindings.binding = 0;
		bindings.stride = sizeof(Vertex);
		bindings.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		return bindings;
	}

	std::array<VkVertexInputAttributeDescription, 2> UBO::Vertex::AttributeDescriptions()
	{
		std::array<VkVertexInputAttributeDescription, 2> attributes = {};
		
		attributes[0].binding = 0;
		attributes[0].location = 0;
		attributes[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributes[0].offset = offsetof(Vertex, pos);
		
		attributes[1].binding = 0;
		attributes[1].location = 1;
		attributes[1].format = VK_FORMAT_R32G32B32_SFLOAT;
		attributes[1].offset = offsetof(Vertex, color);
		
		return attributes;
	}

	std::shared_ptr<UBO> UBO::Create(std::shared_ptr<VKDevice>& device, VkRenderPass& renderPass, VkPipelineCache& pipelineCache)
	{
		return std::make_shared<UBO>(device, renderPass, pipelineCache);
	}

	UBO::UBO(std::shared_ptr<VKDevice>& device, VkRenderPass& renderPass, VkPipelineCache& pipelineCache)
		: VKDrawable(device), mRenderPass(renderPass), mPipelineCache(pipelineCache)
	{
		mVertex = VKShader::Create(mDevice, VKShader::Vertex, "UBO Vertex", "Data/shaders/ubo.vert");
		mFragment = VKShader::Create(mDevice, VKShader::Fragment, "UBO Fragment", "Data/shaders/ubo.frag");

		CreateUniformBuffers();
		CreateDescriptorPool();
		CreateDescriptorSetLayout();
		CreateDescriptorSets();
		CreateGraphicsPipeline(renderPass, pipelineCache);
	}

	UBO::~UBO()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(mDevice->Device(), mBuffers[i], nullptr);
			vkFreeMemory(mDevice->Device(), mBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorSetLayout(mDevice->Device(), mDescriptorSetLayout, nullptr);
		vkDestroyDescriptorPool(mDevice->Device(), mDescriptorPool, nullptr);
		vkDestroyPipelineLayout(mDevice->Device(), mPipelineLayout, nullptr);
		vkDestroyPipeline(mDevice->Device(), mGraphicsPipeline, nullptr);
	}

	void UBO::CreateGraphicsPipeline(VkRenderPass& renderPass, VkPipelineCache& pipelineCache)
	{
		auto binding = Vertex::BindingDescription();
		auto attributes = Vertex::AttributeDescriptions();
		VkPipelineShaderStageCreateInfo stages[] = { mVertex->Stage(), mFragment->Stage() };
		
		VkPipelineVertexInputStateCreateInfo PVISCI = {};
		PVISCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		PVISCI.vertexBindingDescriptionCount = 1;
		PVISCI.pVertexBindingDescriptions = &binding;
		PVISCI.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
		PVISCI.pVertexAttributeDescriptions = attributes.data();
		
		VkPipelineInputAssemblyStateCreateInfo PIASCI = {};
		PIASCI.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		PIASCI.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		PIASCI.primitiveRestartEnable = VK_FALSE;
		
		VkPipelineViewportStateCreateInfo PVSCI = {};
		PVSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		PVSCI.viewportCount = 1;
		PVSCI.scissorCount = 1;
		
		VkPipelineRasterizationStateCreateInfo PRSCI = {};
		PRSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		PRSCI.depthClampEnable = VK_FALSE;
		PRSCI.rasterizerDiscardEnable = VK_FALSE;
		PRSCI.polygonMode = VK_POLYGON_MODE_FILL;
		PRSCI.lineWidth = 1.0f;
		PRSCI.cullMode = VK_CULL_MODE_BACK_BIT;
		PRSCI.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
		PRSCI.depthBiasEnable = VK_FALSE;
		
		VkPipelineMultisampleStateCreateInfo PMSCI = {};
		PMSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		PMSCI.sampleShadingEnable = VK_FALSE;
		PMSCI.minSampleShading = 0.2f; // closer to 1 is smoother
		PMSCI.rasterizationSamples = mDevice->GetMaxUsableSamples();

		VkPipelineDepthStencilStateCreateInfo PDSSCI = {};
		PDSSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		PDSSCI.depthTestEnable = VK_TRUE;
		PDSSCI.depthWriteEnable = VK_TRUE;
		PDSSCI.depthCompareOp = VK_COMPARE_OP_LESS;
		PDSSCI.depthBoundsTestEnable = VK_FALSE;
		PDSSCI.stencilTestEnable = VK_FALSE;
		
		VkPipelineColorBlendAttachmentState PCBAS = {};
		PCBAS.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		PCBAS.blendEnable = VK_FALSE;
		
		VkPipelineColorBlendStateCreateInfo PCBSCI = {};
		PCBSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		PCBSCI.logicOpEnable = VK_FALSE;
		PCBSCI.logicOp = VK_LOGIC_OP_COPY;
		PCBSCI.attachmentCount = 1;
		PCBSCI.pAttachments = &PCBAS;
		PCBSCI.blendConstants[0] = 0.0f;
		PCBSCI.blendConstants[1] = 0.0f;
		PCBSCI.blendConstants[2] = 0.0f;
		PCBSCI.blendConstants[3] = 0.0f;
		
		std::vector<VkDynamicState> dynamicStates =
		{
			VK_DYNAMIC_STATE_VIEWPORT,
			VK_DYNAMIC_STATE_SCISSOR
		};

		VkPipelineDynamicStateCreateInfo PDSCI = {};
		PDSCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		PDSCI.dynamicStateCount = (uint32_t)dynamicStates.size();
		PDSCI.pDynamicStates = dynamicStates.data();

		VkPipelineLayoutCreateInfo PLCI = {};
		PLCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		PLCI.setLayoutCount = 1;
		PLCI.pSetLayouts = &mDescriptorSetLayout;
		VK_ASSERT(vkCreatePipelineLayout(mDevice->Device(), &PLCI, nullptr, &mPipelineLayout), "Failed to create pipeline layout");

		VkGraphicsPipelineCreateInfo GPCI = {};
		GPCI.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		GPCI.stageCount = 2;
		GPCI.pStages = stages;
		GPCI.pVertexInputState = &PVISCI;
		GPCI.pInputAssemblyState = &PIASCI;
		GPCI.pViewportState = &PVSCI;
		GPCI.pRasterizationState = &PRSCI;
		GPCI.pMultisampleState = &PMSCI;
		GPCI.pDepthStencilState = &PDSSCI;
		GPCI.pColorBlendState = &PCBSCI;
		GPCI.pDynamicState = &PDSCI;
		GPCI.layout = mPipelineLayout;
		GPCI.renderPass = renderPass;
		GPCI.subpass = 0;
		GPCI.basePipelineHandle = VK_NULL_HANDLE;
		VK_ASSERT(vkCreateGraphicsPipelines(mDevice->Device(), pipelineCache, 1, &GPCI, nullptr, &mGraphicsPipeline), "Failed to create graphics pipeline");
	}

	void UBO::CreateDescriptorSetLayout()
	{
		VkDescriptorSetLayoutBinding layoutBinding = {};
		layoutBinding.binding = 0;
		layoutBinding.descriptorCount = 1;
		layoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		layoutBinding.pImmutableSamplers = nullptr;
		layoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.pNext = nullptr;
		layoutInfo.flags = 0;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &layoutBinding;
		VK_ASSERT(vkCreateDescriptorSetLayout(mDevice->Device(), &layoutInfo, nullptr, &mDescriptorSetLayout), "Failed to create descriptor set layout");
	}

	void UBO::CreateDescriptorSets()
	{
		std::vector<VkDescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, mDescriptorSetLayout);
		
		VkDescriptorSetAllocateInfo descSetAllocInfo = {};
		descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		descSetAllocInfo.descriptorPool = mDescriptorPool;
		descSetAllocInfo.descriptorSetCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;
		descSetAllocInfo.pSetLayouts = layouts.data();
		
		mDescriptorSets.resize(MAX_FRAMES_IN_FLIGHT);
		VK_ASSERT(vkAllocateDescriptorSets(mDevice->Device(), &descSetAllocInfo, mDescriptorSets.data()), "Failed to create descriptor sets");
		
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			VkDescriptorBufferInfo descBufferInfo = {};
			descBufferInfo.buffer = mBuffers[i];
			descBufferInfo.offset = 0;
			descBufferInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet writeDescSet = {};
			writeDescSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			writeDescSet.dstSet = mDescriptorSets[i];
			writeDescSet.dstBinding = 0;
			writeDescSet.dstArrayElement = 0;
			writeDescSet.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			writeDescSet.descriptorCount = 1;
			writeDescSet.pBufferInfo = &descBufferInfo;

			vkUpdateDescriptorSets(mDevice->Device(), 1, &writeDescSet, 0, nullptr);
		}
	}

	void UBO::CreateDescriptorPool()
	{
		VkDescriptorPoolSize poolSize = {};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = (uint32_t)MAX_FRAMES_IN_FLIGHT;

		VkDescriptorPoolCreateInfo descPoolCI = {};
		descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		descPoolCI.poolSizeCount = 1;
		descPoolCI.pPoolSizes = &poolSize;
		descPoolCI.maxSets = (uint32_t)MAX_FRAMES_IN_FLIGHT;
		VK_ASSERT(vkCreateDescriptorPool(mDevice->Device(), &descPoolCI, nullptr, &mDescriptorPool), "Failed to create descriptor pool");
	}

	void UBO::CreateUniformBuffers()
	{
		VkDeviceSize bufferSize = sizeof(UniformBufferObject);
		mBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		mBuffersMemory.resize(MAX_FRAMES_IN_FLIGHT);
		mBuffersMapped.resize(MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			BufferCreate
			(
				mDevice,
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				bufferSize,
				&mBuffers[i],
				&mBuffersMemory[i]
			);

			vkMapMemory(mDevice->Device(), mBuffersMemory[i], 0, bufferSize, 0, &mBuffersMapped[i]);
		}
	}
}