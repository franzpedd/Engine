#include "Grid.h"

#include "UI/Viewport.h"

namespace Cosmos
{
	Grid::Grid(std::shared_ptr<Renderer>& renderer, Viewport& viewport)
		: Entity("Grid"), mRenderer(renderer), mViewport(viewport)
	{
		Logger() << "Creating Grid";

		CreateResources();
	}

	Grid::~Grid()
	{

	}

	void Grid::OnDraw()
	{
		VkDeviceSize offsets[] = { 0 };
		VkCommandBuffer cmdBuffer = mViewport.GetCommandEntry()->commandBuffers[mRenderer->CurrentFrame()];

		VkDeviceSize vSize = mVertexBuffer->GetSize();
		VkDeviceSize iSize = mIndexBuffer->GetSize();

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &mVertexBuffer->Buffer(), offsets);
		vkCmdBindIndexBuffer(cmdBuffer, mIndexBuffer->Buffer(), 0, VK_INDEX_TYPE_UINT16);
		vkCmdDrawIndexed(cmdBuffer, uint32_t(mIndices.size()), 1, 0, 0, 0);
	}

	void Grid::OnUpdate()
	{

	}

	void Grid::OnDrestroy()
	{
		vkDeviceWaitIdle(mRenderer->BackendDevice()->Device());

		mVertexShader->Destroy();
		mFragmentShader->Destroy();

		vkDestroyDescriptorSetLayout(mRenderer->BackendDevice()->Device(), mDescriptorSetLayout, nullptr);
		vkDestroyPipelineLayout(mRenderer->BackendDevice()->Device(), mPipelineLayout, nullptr);
		vkDestroyPipeline(mRenderer->BackendDevice()->Device(), mGraphicsPipeline, nullptr);

		mIndexBuffer->Destroy();
		mVertexBuffer->Destroy();
	}

	void Grid::CreateResources()
	{
		// create graphics pipeline related (shaders, descriptor set layout, pipeline layout, graphics pipeline)
		{
			mVertexShader = VKShader::Create(mRenderer->BackendDevice(), VKShader::Vertex, "Grid.vert", "Data/shaders/grid.vert");
			mFragmentShader = VKShader::Create(mRenderer->BackendDevice(), VKShader::Fragment, "Grid.frag", "Data/shaders/grid.frag");

			const std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			const std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { mVertexShader->Stage(), mFragmentShader->Stage() };
			const std::array<VkVertexInputBindingDescription, 1> bindings = Vertex::GetBindingDescription();
			const std::array<VkVertexInputAttributeDescription, 2> attributes = Vertex::GetAttributeDescriptions();

			VkDescriptorSetLayoutBinding uboLayoutBinding = {};
			uboLayoutBinding.binding = 0;
			uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			uboLayoutBinding.descriptorCount = 1;
			uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
			uboLayoutBinding.pImmutableSamplers = nullptr;

			VkDescriptorSetLayoutCreateInfo descSetLayoutCI = {};
			descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			descSetLayoutCI.pNext = nullptr;
			descSetLayoutCI.flags = 0;
			descSetLayoutCI.bindingCount = 1;
			descSetLayoutCI.pBindings = &uboLayoutBinding;
			VK_ASSERT(vkCreateDescriptorSetLayout(mRenderer->BackendDevice()->Device(), &descSetLayoutCI, nullptr, &mDescriptorSetLayout), "Failed to create descriptor set layout");

			VkPipelineLayoutCreateInfo pipelineLayoutCI = {};
			pipelineLayoutCI.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
			pipelineLayoutCI.pNext = nullptr;
			pipelineLayoutCI.flags = 0;
			pipelineLayoutCI.setLayoutCount = 1;
			pipelineLayoutCI.pSetLayouts = &mDescriptorSetLayout;
			VK_ASSERT(vkCreatePipelineLayout(mRenderer->BackendDevice()->Device(), &pipelineLayoutCI, nullptr, &mPipelineLayout), "Failed to create descriptor set layout");

			VkPipelineVertexInputStateCreateInfo VISCI = {};
			VISCI.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
			VISCI.pNext = nullptr;
			VISCI.flags = 0;
			VISCI.vertexBindingDescriptionCount = (uint32_t)bindings.size();
			VISCI.pVertexBindingDescriptions = bindings.data();
			VISCI.vertexAttributeDescriptionCount = (uint32_t)attributes.size();
			VISCI.pVertexAttributeDescriptions = attributes.data();

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
			RSCI.cullMode = VK_CULL_MODE_BACK_BIT;
			RSCI.frontFace = VK_FRONT_FACE_CLOCKWISE;
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
			pipelineCI.renderPass = mViewport.GetCommandEntry()->renderPass;
			pipelineCI.subpass = 0;
			pipelineCI.basePipelineHandle = VK_NULL_HANDLE;
			VK_ASSERT(vkCreateGraphicsPipelines(mRenderer->BackendDevice()->Device(), mRenderer->PipelineCache(), 1, &pipelineCI, nullptr, &mGraphicsPipeline), "Failed to create graphics pipeline");
		}

		// create vertex buffer
		mVertexBuffer = VKBuffer::Create
		(
			mRenderer->BackendDevice(),
			VKBuffer::Type::Vertex,
			sizeof(mVertices[0]) * mVertices.size(),
			mViewport.GetCommandEntry()->commandPool,
			mVertices.data()
		);

		// create index buffer
		mIndexBuffer = VKBuffer::Create
		(
			mRenderer->BackendDevice(),
			VKBuffer::Type::Index,
			sizeof(mIndices[0]) * mIndices.size(),
			mViewport.GetCommandEntry()->commandPool,
			mIndices.data()
		);
	}
}