#include "epch.h"
#include "VKBuffer.h"

#include "VKDevice.h"

namespace Cosmos
{
	VkResult BufferCreate(std::shared_ptr<VKDevice> device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
	{
		// specify buffer
		VkBufferCreateInfo bufferCI = {};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = size;
		bufferCI.usage = usage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_ASSERT(vkCreateBuffer(device->GetDevice(), &bufferCI, nullptr, buffer), "Failed to create buffer");

		// alocate memory for specified buffer 
		VkMemoryRequirements memoryReqs;
		vkGetBufferMemoryRequirements(device->GetDevice(), *buffer, &memoryReqs);

		VkMemoryAllocateInfo memoryAllocInfo = {};
		memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocInfo.pNext = nullptr;
		memoryAllocInfo.allocationSize = memoryReqs.size;
		memoryAllocInfo.memoryTypeIndex = device->GetMemoryType(memoryReqs.memoryTypeBits, properties);
		VK_ASSERT(vkAllocateMemory(device->GetDevice(), &memoryAllocInfo, nullptr, memory), "Failed to allocate memory for buffer");

		// data is not null, must map the buffer and copy the data
		if (data != nullptr)
		{
			void* mapped;
			VK_ASSERT(vkMapMemory(device->GetDevice(), *memory, 0, size, 0, &mapped), "Faled to map memory");
			memcpy(mapped, data, size);

			// if host coherency hasn't been requested, do a manual flush to make writes visible
			if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange = {};
				mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				vkFlushMappedMemoryRanges(device->GetDevice(), 1, &mappedRange);
			}

			vkUnmapMemory(device->GetDevice(), *memory);
		}

		// link buffer with allocated memory
		VK_ASSERT(vkBindBufferMemory(device->GetDevice(), *buffer, *memory, 0), "Failed to bind buffer with memory");

		return VK_SUCCESS;
	}

	VkCommandBuffer BeginSingleTimeCommand(std::shared_ptr<VKDevice> device, VkCommandPool& commandPool)
	{
		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandPool = commandPool;
		cmdBufferAllocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device->GetDevice(), &cmdBufferAllocInfo, &commandBuffer);

		VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);

		return commandBuffer;
	}

	void EndSingleTimeCommand(std::shared_ptr<VKDevice> device, VkCommandPool& commandPool, VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(device->GetGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device->GetGraphicsQueue());

		vkFreeCommandBuffers(device->GetDevice(), commandPool, 1, &commandBuffer);
	}

	VkCommandBuffer CreateCommandBuffer(std::shared_ptr<VKDevice> device, VkCommandPool& cmdPool, VkCommandBufferLevel level, bool begin)
	{
		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.pNext = nullptr;
		cmdBufferAllocInfo.commandBufferCount = 1;
		cmdBufferAllocInfo.commandPool = cmdPool;
		cmdBufferAllocInfo.level = level;

		VkCommandBuffer cmdBuffer;
		VK_ASSERT(vkAllocateCommandBuffers(device->GetDevice(), &cmdBufferAllocInfo, &cmdBuffer), "Failed to allocate command buffers");

		if (begin)
		{
			VkCommandBufferBeginInfo cmdBufferBI = {};
			cmdBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			cmdBufferBI.pNext = nullptr;
			cmdBufferBI.flags = 0;
			VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBI), "Failed to initialize command buffer");
		}

		return cmdBuffer;
	}

	void BeginCommandBuffer(VkCommandBuffer cmdBuffer)
	{
		VkCommandBufferBeginInfo cmdBufferBI = {};
		cmdBufferBI.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBI.pNext = nullptr;
		cmdBufferBI.flags = 0;
		VK_ASSERT(vkBeginCommandBuffer(cmdBuffer, &cmdBufferBI), "Failed to initialize command buffer");
	}

	void FlushCommandBuffer(std::shared_ptr<VKDevice> device, VkCommandPool& cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue, bool free)
	{
		VK_ASSERT(vkEndCommandBuffer(cmdBuffer), "Failed to end the recording of the command buffer");

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;

		VkFenceCreateInfo fenceCI = {};
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.pNext = nullptr;
		fenceCI.flags = 0;

		VkFence fence;
		VK_ASSERT(vkCreateFence(device->GetDevice(), &fenceCI, nullptr, &fence), "Failed to create fence for command buffer submission");
		VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, fence), "Failed to submit command buffer");
		VK_ASSERT(vkWaitForFences(device->GetDevice(), 1, &fence, VK_TRUE, 100000000000), "Failed to wait for fences");

		vkDestroyFence(device->GetDevice(), fence, nullptr);

		if (free)
		{
			vkFreeCommandBuffers(device->GetDevice(), cmdPool, 1, &cmdBuffer);
		}
	}
}