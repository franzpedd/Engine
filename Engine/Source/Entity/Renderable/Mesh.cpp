#include "epch.h"
#include "Mesh.h"

#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VKBuffer.h"

namespace Cosmos
{
	Mesh::Mesh(std::shared_ptr<Renderer> renderer, std::vector<Vertex> vertices, std::vector<uint32_t> indices)
		: mRenderer(renderer), mVertices(vertices), mIndices(indices)
	{
		CreateResources();
	}

	void Mesh::Draw(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkDescriptorSet& descriptorSet)
	{
		VkDeviceSize offsets[] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertexBuffer, offsets);

		if (mIndices.size() > 0)
		{
			vkCmdBindIndexBuffer(commandBuffer, mIndexBuffer, 0, VK_INDEX_TYPE_UINT32);
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
			vkCmdDrawIndexed(commandBuffer, (uint32_t)mIndices.size(), 1, 0, 0, 0);
		}

		else
		{
			vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptorSet, 0, nullptr);
			vkCmdDraw(commandBuffer, (uint32_t)mVertices.size(), 1, 0, 0);
		}
	}

	void Mesh::CreateResources()
	{
		// vertex Buffer
		{
			VkDeviceSize bufferSize = sizeof(mVertices[0]) * mVertices.size();

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
					mVertices.data()
				),
				"Failed to create model Vertex Buffer"
			);
		}

		// index buffer
		{
			if (mIndices.size() > 0)
			{
				VkDeviceSize bufferSize = sizeof(mIndices[0]) * mIndices.size();

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
						mIndices.data()
					),
					"Failed to create model Vertex Buffer"
				);
			}
		}
	}

	void Mesh::DestroyResources()
	{
		vkDestroyBuffer(mRenderer->GetDevice()->GetDevice(), mVertexBuffer, nullptr);
		vkFreeMemory(mRenderer->GetDevice()->GetDevice(), mVertexMemory, nullptr);

		if (mIndices.size() > 0)
		{
			vkDestroyBuffer(mRenderer->GetDevice()->GetDevice(), mIndexBuffer, nullptr);
			vkFreeMemory(mRenderer->GetDevice()->GetDevice(), mIndexMemory, nullptr);
		}

		mIndices.clear();
		mVertices.clear();
	}
}