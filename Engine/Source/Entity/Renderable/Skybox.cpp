#include "epch.h"
#include "Skybox.h"

#include "Renderer/Vulkan/VKRenderer.h"
#include "Util/FileSystem.h"

namespace Cosmos
{
	Skybox::Skybox(Shared<Renderer> renderer, Shared<Camera> camera)
		: mRenderer(renderer), mCamera(camera)
	{
		// loading mesh
		mVertices =
		{
			{-1.0f, -1.0f, -1.0f}, // 0: Bottom-back-left
			{ 1.0f, -1.0f, -1.0f}, // 1: Bottom-back-right
			{ 1.0f,  1.0f, -1.0f}, // 2: Top-back-right
			{-1.0f,  1.0f, -1.0f}, // 3: Top-back-left
			{-1.0f, -1.0f,  1.0f}, // 4: Bottom-front-left
			{ 1.0f, -1.0f,  1.0f}, // 5: Bottom-front-right
			{ 1.0f,  1.0f,  1.0f}, // 6: Tosp-front-right
			{-1.0f,  1.0f,  1.0f}  // 7: Top-front-left
		};

		mIndices =
		{
			4, 5, 6, 6, 7, 4, // Front face
			0, 3, 2, 2, 1, 0, // Back face
			0, 4, 7, 7, 3, 0, // Left face
			1, 2, 6, 6, 5, 1, // Right face
			3, 7, 6, 6, 2, 3, // Top face
			0, 1, 5, 5, 4, 0  // Bottom face
		};

		// vertex Buffer
		VkDeviceSize bufferSize = sizeof(mVertices[0]) * mVertices.size();
		
		VK_ASSERT
		(
			BufferCreate
			(
				std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				bufferSize,
				&mVertexBuffer,
				&mVertexMemory,
				mVertices.data()
			),
			"Failed to create model Vertex Buffer"
		);
		
		// index buffer
		bufferSize = sizeof(mIndices[0]) * mIndices.size();

		VK_ASSERT
		(
			BufferCreate
			(
				std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(),
				VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				bufferSize,
				&mIndexBuffer,
				&mIndexMemory,
				mIndices.data()
			),
			"Failed to create model Vertex Buffer"
		);

		// default sky
		mPaths[0] = GetAssetSubDir("Textures/skybox/right.jpg");	// +X
		mPaths[1] = GetAssetSubDir("Textures/skybox/left.jpg");		// -X
		mPaths[2] = GetAssetSubDir("Textures/skybox/top.jpg");		// +Y
		mPaths[3] = GetAssetSubDir("Textures/skybox/bottom.jpg");	// -Y
		mPaths[4] = GetAssetSubDir("Textures/skybox/front.jpg");	// +Z
		mPaths[5] = GetAssetSubDir("Textures/skybox/back.jpg");		// -Z

		LoadSkybox();
		mSkyboxModel = CreateShared<Model>(mRenderer, mCamera);
		mSkyboxModel->LoadFromFile(GetAssetSubDir("Models/skybox.gltf"));
	}

	void Skybox::LoadSkybox()
	{
		// clear previous skybox
		if (!mCubemap) mCubemap.reset();

		// create cubemap
		mCubemap = TextureCubemap::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), mPaths, MSAA::SAMPLE_1_BIT);

		// create vulkan resources
		CreateResources();
	}

	void Skybox::OnUpdate(float deltaTime, glm::mat4 transform)
	{
		UniformBufferObject ubo = {};
		ubo.model = transform;
		ubo.view = mCamera->GetViewRef();
		ubo.proj = mCamera->GetProjectionRef();
		
		memcpy(mUniformBuffersMapped[mRenderer->GetCurrentFrame()], &ubo, sizeof(ubo));
	}

	void Skybox::OnRender(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetPipelinesRef()["Skybox"]->GetPipeline());
		mSkyboxModel->OnRenderSkybox
		(
			commandBuffer,
			std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetPipelinesRef()["Skybox"]->GetPipelineLayout(), 
			mDescriptorSets[mRenderer->GetCurrentFrame()]
		);
		
		//VkDeviceSize offsets[] = { 0 };

		//vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertexBuffer, offsets);

		//if (mIndices.size() > 0)
		//{
		//	vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
		//	vkCmdBindDescriptorSets
		//	(
		//		commandBuffer, 
		//		VK_PIPELINE_BIND_POINT_GRAPHICS, 
		//		std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetPipelinesRef()["Skybox"]->GetPipelineLayout(), 
		//		0, 
		//		1, 
		//		&mDescriptorSets[mRenderer->GetCurrentFrame()],
		//		0, 
		//		nullptr
		//	);
		//	vkCmdDrawIndexed(commandBuffer, (uint32_t)mIndices.size(), 1, 0, 0, 0);
		//}
	}

	void Skybox::Destroy()
	{
		// cubemap
		vkDeviceWaitIdle(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice());

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mUniformBuffers[i], nullptr);
			vkFreeMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mUniformBuffersMemory[i], nullptr);
		}

		vkDestroyDescriptorPool(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mDescriptorPool, nullptr);

		if (mCubemap)
			mCubemap.reset();

		mSkyboxModel->Destroy();

		//// model
		//vkDestroyBuffer(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mVertexBuffer, nullptr);
		//vkFreeMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mVertexMemory, nullptr);
		//
		//if (mIndices.size() > 0)
		//{
		//	vkDestroyBuffer(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mIndexBuffer, nullptr);
		//	vkFreeMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mIndexMemory, nullptr);
		//}
		//
		//mIndices.clear();
		//mVertices.clear();
	}

	void Skybox::CreateResources()
	{
		// ubo
		{
			mUniformBuffers.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			mUniformBuffersMemory.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			mUniformBuffersMapped.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);

			for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
			{
				// camera's ubo
				BufferCreate
				(
					std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(),
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					sizeof(UniformBufferObject),
					&mUniformBuffers[i],
					&mUniformBuffersMemory[i]
				);

				vkMapMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mUniformBuffersMemory[i], 0, sizeof(UniformBufferObject), 0, &mUniformBuffersMapped[i]);
			}
		}

		// create descriptor pool and descriptor sets
		{
			std::array<VkDescriptorPoolSize, 2> poolSizes = {};
			poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			poolSizes[0].descriptorCount = 2;
			poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			poolSizes[1].descriptorCount = 2;

			VkDescriptorPoolCreateInfo descPoolCI = {};
			descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			descPoolCI.poolSizeCount = (uint32_t)poolSizes.size();
			descPoolCI.pPoolSizes = poolSizes.data();
			descPoolCI.maxSets = 2;
			VK_ASSERT(vkCreateDescriptorPool(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), &descPoolCI, nullptr, &mDescriptorPool), "Failed to create descriptor pool");

			std::vector<VkDescriptorSetLayout> layouts
			(
				RENDERER_MAX_FRAMES_IN_FLIGHT,
				std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetPipelinesRef()["Skybox"]->GetDescriptorSetLayout()
			);

			VkDescriptorSetAllocateInfo descSetAllocInfo = {};
			descSetAllocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			descSetAllocInfo.descriptorPool = mDescriptorPool;
			descSetAllocInfo.descriptorSetCount = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;
			descSetAllocInfo.pSetLayouts = layouts.data();

			mDescriptorSets.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
			VK_ASSERT(vkAllocateDescriptorSets(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), &descSetAllocInfo, mDescriptorSets.data()), "Failed to allocate descriptor sets");

			UpdateDescriptorSets();
		}
	}

	void Skybox::UpdateDescriptorSets()
	{
		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			std::vector<VkWriteDescriptorSet> descriptorWrites = {};

			//
			VkDescriptorBufferInfo cameraUBOInfo = {};
			cameraUBOInfo.buffer = mUniformBuffers[i];
			cameraUBOInfo.offset = 0;
			cameraUBOInfo.range = sizeof(UniformBufferObject);

			VkWriteDescriptorSet cameraUBODesc = {};
			cameraUBODesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			cameraUBODesc.dstSet = mDescriptorSets[i];
			cameraUBODesc.dstBinding = 0;
			cameraUBODesc.dstArrayElement = 0;
			cameraUBODesc.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			cameraUBODesc.descriptorCount = 1;
			cameraUBODesc.pBufferInfo = &cameraUBOInfo;

			descriptorWrites.push_back(cameraUBODesc);

			//
			VkDescriptorImageInfo cubemapInfo = {};
			cubemapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			cubemapInfo.imageView = mCubemap->GetView();
			cubemapInfo.sampler = mCubemap->GetSampler();

			VkWriteDescriptorSet albedoDesc = {};
			albedoDesc.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			albedoDesc.dstSet = mDescriptorSets[i];
			albedoDesc.dstBinding = 1;
			albedoDesc.dstArrayElement = 0;
			albedoDesc.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			albedoDesc.descriptorCount = 1;
			albedoDesc.pImageInfo = &cubemapInfo;

			descriptorWrites.push_back(albedoDesc);

			vkUpdateDescriptorSets(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), (uint32_t)descriptorWrites.size(), descriptorWrites.data(), 0, nullptr);
		}
	}
}