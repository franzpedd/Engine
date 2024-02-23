#include "epch.h"
#include "Terrain.h"

#include "Vertex.h"
#include "Renderer/Texture.h"
#include "Renderer/Vulkan/VKBuffer.h"
#include "Util/FileSystem.h"

namespace Cosmos::renderable
{
	Terrain::Terrain(std::shared_ptr<Device> device, std::string path, uint32_t width, uint32_t height, uint32_t divisions)
		: mDevice(device), mPath(path), mWidth(width), mHeight(height), mDivisions(divisions)
	{
		CreateTexture();
		CreatePlaneMesh(width, height, divisions);
		PrepareUniformBuffer();
		SetupDescriptors();
	}

	void Terrain::Destroy()
	{
		vkDestroyBuffer(mDevice->GetDevice(), mVertexBuffer, nullptr);
		vkFreeMemory(mDevice->GetDevice(), mVertexMemory, nullptr);
		vkDestroyBuffer(mDevice->GetDevice(), mIndexBuffer, nullptr);
		vkFreeMemory(mDevice->GetDevice(), mIndexMemory, nullptr);

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			vkDestroyBuffer(mDevice->GetDevice(), mUniformBuffer[i], nullptr);
			vkFreeMemory(mDevice->GetDevice(), mUniformBuffersMemory[i], nullptr);
		}
	}

	void Terrain::CreateTexture()
	{
		mHeightmap = Texture2D::Create(mDevice, mPath.c_str(), (Texture2D::MSAA)Commander::Get().GetPrimary()->msaa);

	}

	void Terrain::CreatePlaneMesh(uint32_t width, uint32_t height, uint32_t divisions)
	{
		if (width == 0 || height == 0)
			return;

		constexpr float innerSpace = 1.0f;
		constexpr float patchSize = 64.0f;
		constexpr const float wx = 2.0f;
		constexpr const float wz = 2.0f;
		constexpr const float uvScale = 1.0f;
		
		// vertices
		const uint32_t vertexCount = (uint32_t)patchSize * (uint32_t)patchSize;
		Vertex* vertices = new Vertex[vertexCount];

		for (uint32_t x = 0; x < width; x++)
		{
			for (uint32_t z = 0; z < height; z++)
			{
				uint32_t index = x + (z * (uint32_t)patchSize);

				vertices[index].position[0] = (x * wx) + (wx / 2.0f) - (patchSize * wx / 2.0f);
				vertices[index].position[1] = 0.0f;
				vertices[index].position[2] = (z * wz) + (wz / 2.0f) - (patchSize * wz / 2.0f);
				vertices[index].uv0 = glm::vec2((float)x / (patchSize - 1), (float)z / (patchSize - 1)) * uvScale;
			}
		}

		// indices
		constexpr uint32_t w = ((uint32_t)patchSize - 1);
		constexpr uint32_t indexCount = w * w * 4;
		uint32_t* indices = new uint32_t[indexCount];

		for (uint32_t x = 0; x < w; x++)
		{
			for (uint32_t y = 0; y < w; y++)
			{
				uint32_t index = (x + (y * w)) * 4;

				indices[index] = x + (y * (uint32_t)patchSize);
				indices[index + 1] = indices[index] + (uint32_t)patchSize;
				indices[index + 2] = indices[index + 1] + 1;
				indices[index + 3] = indices[index] + 1;
			}
		}

		mIndicesCount = indexCount;
		uint32_t vertexBufferSize = vertexCount * sizeof(Vertex);
		uint32_t indexBufferSize = indexCount * sizeof(uint32_t);

		// staging buffers
		VkBuffer stagingVertexbuffer;
		VkDeviceMemory stagingVertexmemory;
		VkBuffer stagingIndexbuffer;
		VkDeviceMemory stagingIndexmemory;

		VK_ASSERT(BufferCreate
		(
			mDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			vertexBufferSize,
			&stagingVertexbuffer,
			&stagingVertexmemory,
			vertices), 
			"Error when creating vertex staging buffer"
		);

		VK_ASSERT(BufferCreate
		(
			mDevice,
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			indexBufferSize,
			&stagingIndexbuffer,
			&stagingIndexmemory,
			vertices), 
			"Error when creating index staging buffer"
		);

		// destiny buffers 
		VK_ASSERT(BufferCreate
		(
			mDevice,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			vertexBufferSize,
			&mVertexBuffer,
			&mVertexMemory),
			"Error when creating destiny vertex buffer"
		);

		VK_ASSERT(BufferCreate
		(
			mDevice,
			VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			indexBufferSize,
			&mIndexBuffer,
			&mIndexMemory),
			"Error when creating destiny index buffer"
		);

		// copy staging buffers to destiny ones
		VkCommandBuffer cmd = CreateCommandBuffer(
			mDevice,
			Commander::Get().GetPrimary()->commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			true
		);

		VkBufferCopy copyRegion = {};

		copyRegion.size = vertexBufferSize;
		vkCmdCopyBuffer(cmd, stagingVertexbuffer, mVertexBuffer, 1, &copyRegion);

		copyRegion.size = indexBufferSize;
		vkCmdCopyBuffer(cmd, stagingIndexbuffer, mIndexBuffer, 1, &copyRegion);

		FlushCommandBuffer(mDevice, Commander::Get().GetPrimary()->commandPool, cmd, mDevice->GetGraphicsQueue(), true);

		// free all used resources
		vkDestroyBuffer(mDevice->GetDevice(), stagingVertexbuffer, nullptr);
		vkFreeMemory(mDevice->GetDevice(), stagingVertexmemory, nullptr);
		vkDestroyBuffer(mDevice->GetDevice(), stagingIndexbuffer, nullptr);
		vkFreeMemory(mDevice->GetDevice(), stagingIndexmemory, nullptr);

		delete[] vertices;
		delete[] indices;
	}

	void Terrain::PrepareUniformBuffer()
	{
		mUniformBuffer.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
		mUniformBuffersMemory.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);
		mUniformBuffersMapped.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);

		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			VK_ASSERT
			(
				BufferCreate
				(
					mDevice,
					VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					sizeof(UniformBufferTesselation),
					&mUniformBuffer[i],
					&mUniformBuffersMemory[i]
				),
				" Failed to create buffer"
			);

			vkMapMemory(mDevice->GetDevice(), mUniformBuffersMemory[i], 0, sizeof(UniformBufferTesselation), 0, &mUniformBuffersMapped[i]);
		}
	}

	void Terrain::SetupDescriptors()
	{
		// descriptor pool
		//std::array<VkDescriptorPoolSize, 2> poolSizes = {};
		//poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		//poolSizes[0].descriptorCount = 2;
		//poolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		//poolSizes[1].descriptorCount = 2;
		//
		//VkDescriptorPoolCreateInfo descPoolCI = {};
		//descPoolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		//descPoolCI.pNext = nullptr;
		//descPoolCI.flags = 0;
		//descPoolCI.poolSizeCount = (uint32_t)poolSizes.size();
		//descPoolCI.pPoolSizes = poolSizes.data();
		//descPoolCI.maxSets = (uint32_t)RENDERER_MAX_FRAMES_IN_FLIGHT;
		//VK_ASSERT(vkCreateDescriptorPool(mDevice->GetDevice(), &descPoolCI, nullptr, &mDescriptorPool), "Failed to create descriptor pool");
	
		// // // descriptor layout
		// // std::array<VkDescriptorSetLayoutBinding, 2> bindings = {};
		// // // tesselation
		// // bindings[0].binding = 0;
		// // bindings[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		// // bindings[0].stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		// // bindings[0].descriptorCount = 1;
		// // bindings[0].pImmutableSamplers = nullptr;
		// // // height map
		// // bindings[1].binding = 1;
		// // bindings[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		// // bindings[1].stageFlags = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT | VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
		// // bindings[1].descriptorCount = 1;
		// // bindings[1].pImmutableSamplers = nullptr;

		// // VkDescriptorSetLayoutCreateInfo descSetLayoutCI = {};
		// // descSetLayoutCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		// // descSetLayoutCI.pNext = nullptr;
		// // descSetLayoutCI.flags = 0;
		// // descSetLayoutCI.pBindings = bindings.data();
		// // descSetLayoutCI.bindingCount = (uint32_t)bindings.size();
		// //VK_ASSERT(vkCreateDescriptorSetLayout(mDevice->GetDevice(), &descSetLayoutCI, nullptr, &mDescriptorSetLayout), "Failed to create descriptor set layout");
		
		// descritpor sets
		for (size_t i = 0; i < RENDERER_MAX_FRAMES_IN_FLIGHT; i++)
		{
			std::array<VkWriteDescriptorSet, 3> writeDescriptorSets = {};

			// tesselation
			{
				VkDescriptorBufferInfo uboInfo = {};
				uboInfo.buffer = mUniformBuffer[i];
				uboInfo.offset = 0;
				uboInfo.range = sizeof(UniformBufferTesselation);

				writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[0].pNext = nullptr;
				writeDescriptorSets[0].dstSet = mDescriptorSets[i];
				writeDescriptorSets[0].dstBinding = 0;
				writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
				writeDescriptorSets[0].pBufferInfo = &uboInfo;
			}

			// height map texture
			{
				VkDescriptorImageInfo heightMapInfo = {};
				heightMapInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				heightMapInfo.imageView = mHeightmap->GetView();
				heightMapInfo.sampler = mHeightmap->GetSampler();

				writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				writeDescriptorSets[1].pNext = nullptr;
				writeDescriptorSets[1].dstSet = mDescriptorSets[i];
				writeDescriptorSets[1].dstBinding = 1;
				writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				writeDescriptorSets[1].pImageInfo = &heightMapInfo;
			}

			vkUpdateDescriptorSets(mDevice->GetDevice(), (uint32_t)writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
		}
	}
}