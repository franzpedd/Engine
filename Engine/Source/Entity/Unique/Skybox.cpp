#include "epch.h"
#include "Skybox.h"

#include "Renderer/Vulkan/VKRenderer.h"
#include "Util/FileSystem.h"

#include "wrapper_assimp.h"

namespace Cosmos
{
	Skybox::Skybox(Shared<Renderer> renderer, Shared<Camera> camera)
		: mRenderer(renderer), mCamera(camera)
	{
		mCubemodel = CreateShared<Model>(mRenderer, mCamera);
		mCubemodel->LoadFromFile(GetAssetSubDir("Models/skybox.gltf"));

		// default skybox texture
		mPaths[0] = GetAssetSubDir("Textures/skybox/right.jpg");	// +X
		mPaths[1] = GetAssetSubDir("Textures/skybox/left.jpg");		// -X
		mPaths[2] = GetAssetSubDir("Textures/skybox/top.jpg");		// +Y
		mPaths[3] = GetAssetSubDir("Textures/skybox/bottom.jpg");	// -Y
		mPaths[4] = GetAssetSubDir("Textures/skybox/front.jpg");	// +Z
		mPaths[5] = GetAssetSubDir("Textures/skybox/back.jpg");		// -Z

		LoadSkybox();
	}

	Skybox::~Skybox()
	{
		vkDeviceWaitIdle(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice());
		
		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mUniformBuffers[i], nullptr);
			vkFreeMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mUniformBuffersMemory[i], nullptr);
		}
		
		vkDestroyDescriptorPool(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mDescriptorPool, nullptr);

		mCubemodel->Destroy();
	}

	void Skybox::LoadSkybox()
	{
		// clear previous skybox
		if (!mCubemap) mCubemap.reset();
		
		// create cubemap
		mCubemap = TextureCubemap::Create(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), mPaths, MSAA::SAMPLE_1_BIT);
		
		// create renderer resources
		CreateResources();
	}

	void Skybox::OnUpdate(float deltaTime)
	{
		ModelViewProjection_BufferObject ubo = {};
		ubo.model = glm::mat4(1.0f); // sky doesnt move around, so identity matrix will do
		ubo.view = mCamera->GetViewRef();
		ubo.proj = mCamera->GetProjectionRef();
		
		memcpy(mUniformBuffersMapped[mRenderer->GetCurrentFrame()], &ubo, sizeof(ubo));
	}

	void Skybox::OnRender(VkCommandBuffer commandBuffer)
	{
		vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetPipelinesRef()["Skybox"]->GetPipeline());

		for (auto mesh : mCubemodel->GetMeshesRef())
		{
			mesh.Draw(commandBuffer, std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetPipelinesRef()["Skybox"]->GetPipelineLayout(), mDescriptorSets[mRenderer->GetCurrentFrame()]);
		}
	}

	void Skybox::CreateResources()
	{
		// ubo's
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
					sizeof(ModelViewProjection_BufferObject),
					&mUniformBuffers[i],
					&mUniformBuffersMemory[i]
				);
		
				vkMapMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mUniformBuffersMemory[i], 0, sizeof(ModelViewProjection_BufferObject), 0, &mUniformBuffersMapped[i]);
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
			cameraUBOInfo.range = sizeof(ModelViewProjection_BufferObject);
		
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