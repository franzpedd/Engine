#include "epch.h"
#include "Model.h"

#include "Entity/Camera.h"
#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VKBuffer.h"
#include "Renderer/Vulkan/VKShader.h"

namespace Cosmos
{
	Mesh::Mesh(std::vector<VKVertex> vertices, std::vector<uint32_t> indices)
		: mVertices(vertices), mIndices(indices)
	{
	}

	Model::Model(std::shared_ptr<Renderer>& renderer, Camera& camera)
		: mRenderer(renderer), mCamera(camera)
	{
	}

	void Model::Draw(VkCommandBuffer commandBuffer)
	{
		uint32_t currentFrame = mRenderer->CurrentFrame();
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline);
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertexBuffer, offsets);

		if (mMeshes[0].GetIndicesRef().size() > 0)
		{
			vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[currentFrame], 0, nullptr);
			vkCmdDrawIndexed(commandBuffer, (uint32_t)mMeshes[0].GetIndicesRef().size(), 1, 0, 0, 0);
		}
		
		else
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mPipelineLayout, 0, 1, &mDescriptorSets[currentFrame], 0, nullptr);
			vkCmdDraw(commandBuffer, (uint32_t)mMeshes[0].GetVerticesRef().size(), 1, 0, 0);
		}
	}

	void Model::Update(float deltaTime)
	{
		UniformBufferObject ubo = {};
		ubo.model = glm::mat4(1.0f);
		ubo.view = mCamera.GetView();
		ubo.proj = mCamera.GetProjection();

		memcpy(mUniformBuffersMapped[mRenderer->CurrentFrame()], &ubo, sizeof(ubo));
	}

	void Model::Destroy()
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

		vkDestroyBuffer(mRenderer->GetDevice()->GetDevice(), mVertexBuffer, nullptr);
		vkFreeMemory(mRenderer->GetDevice()->GetDevice(), mVertexMemory, nullptr);

		if (mMeshes[0].GetIndicesRef().size() > 0)
		{
			vkDestroyBuffer(mRenderer->GetDevice()->GetDevice(), mIndexBuffer, nullptr);
			vkFreeMemory(mRenderer->GetDevice()->GetDevice(), mIndexMemory, nullptr);
		}

		mMeshes[0].GetIndicesRef().clear();
		mMeshes[0].GetVerticesRef().clear();
		mMeshes.clear();
	}

	void Model::LoadFromFile(std::string path)
	{
		if (mLoaded)
			Destroy();

		Assimp::Importer importer;
		uint32_t flags = aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_CalcTangentSpace;

		const aiScene* scene = importer.ReadFile(path.c_str(), flags);

		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
		{
			LOG_TO_TERMINAL(Logger::Error, "Could not load model %s. Error: %s", path.c_str(), importer.GetErrorString());
			return;
		}

		ProcessNode(scene->mRootNode, scene);

		mLoaded = true;
		mPath = path;

		CreateResources();
	}

	Mesh Model::ProcessMesh(aiMesh* mesh, const aiScene* scene)
	{
		std::vector<VKVertex> vertices;
		std::vector<uint32_t> indices;

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			VKVertex vertex;

			// position
			vertex.position = glm::vec3(mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z);

			// color
			if(mesh->mColors[0])
				vertex.color = glm::vec3(mesh->mColors[0]->r, mesh->mColors[0]->g, mesh->mColors[0]->b);
			else
				vertex.color = glm::vec3(1.0f, 1.0f, 1.0f);

			// uv0
			if (mesh->mTextureCoords[0])
				vertex.uv0 = glm::vec2(mesh->mTextureCoords[0][i].x, mesh->mTextureCoords[0][i].y);
			else
				vertex.uv0 = glm::vec2(0.0f, 0.0f);

			vertices.push_back(vertex);
		}

		for (uint32_t i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];

			for (uint32_t j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		return Mesh(vertices, indices);
	}

	void Model::ProcessNode(aiNode* node, const aiScene* scene)
	{
		if (node->mNumMeshes > 1)
			LOG_TO_TERMINAL(Logger::Error, "Model with more than one mesh is not yet fully implemented");

		for (uint32_t i = 0; i < node->mNumMeshes; i++)
			mMeshes.push_back(ProcessMesh(scene->mMeshes[node->mMeshes[i]], scene));

		for (uint32_t i = 0; i < node->mNumChildren; i++)
			ProcessNode(node->mChildren[i], scene);
	}

	void Model::CreateResources()
	{
		std::shared_ptr<VKShader> vShader = VKShader::Create(std::reinterpret_pointer_cast<VKDevice>(mRenderer->GetDevice()), VKShader::Vertex, "Model.vert", "Data/Shaders/model.vert");
		std::shared_ptr<VKShader> fShader = VKShader::Create(std::reinterpret_pointer_cast<VKDevice>(mRenderer->GetDevice()), VKShader::Fragment, "Model.frag", "Data/Shaders/model.frag");

		// create graphics pipeline related (shaders, descriptor set layout, pipeline layout, graphics pipeline)
		{
			const std::vector<VkDynamicState> dynamicStates = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
			const std::vector<VkPipelineShaderStageCreateInfo> shaderStages = { vShader->Stage(), fShader->Stage() };

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
			VISCI.vertexAttributeDescriptionCount = (uint32_t)VKVertex::GetAttributeDescriptions().size();
			VISCI.pVertexAttributeDescriptions = VKVertex::GetAttributeDescriptions().data();
			VISCI.vertexBindingDescriptionCount = (uint32_t)VKVertex::GetBindingDescription().size();
			VISCI.pVertexBindingDescriptions = VKVertex::GetBindingDescription().data();

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
			pipelineCI.renderPass = mRenderer->GetCommander().AccessMainCommandEntry()->renderPass;
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

		// vertex Buffer
		{
			auto& vertices = mMeshes[0].GetVerticesRef();
			VkDeviceSize bufferSize =  sizeof(vertices[0]) * vertices.size();

			VK_ASSERT
			(
				BufferCreate
				(
					mRenderer->GetDevice(),
					VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					bufferSize,
					&mVertexBuffer,
					&mVertexMemory,
					mMeshes[0].GetVerticesRef().data()
				),
				"Failed to create model Vertex Buffer"
			);
		}

		// index buffer
		{
			auto& indices = mMeshes[0].GetIndicesRef();

			if (indices.size() > 0)
			{
				VkDeviceSize bufferSize = sizeof(indices[0]) * indices.size();
			
				VK_ASSERT
				(
					BufferCreate
					(
						mRenderer->GetDevice(),
						VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
						VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
						bufferSize,
						&mIndexBuffer,
						&mIndexMemory,
						mMeshes[0].GetIndicesRef().data()
					),
					"Failed to create model Vertex Buffer"
				);
			}
		}

		// destroy the shader module after usage
		vkDestroyShaderModule(mRenderer->GetDevice()->GetDevice(), vShader->Module(), nullptr);
		vkDestroyShaderModule(mRenderer->GetDevice()->GetDevice(), fShader->Module(), nullptr);
	}
}