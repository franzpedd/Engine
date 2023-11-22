#include "epch.h"
#include "VKBuffer.h"

#include "VKDevice.h"

namespace Cosmos
{
	std::shared_ptr<VKBuffer> VKBuffer::Create(std::shared_ptr<VKDevice>& device, Type type, VkDeviceSize size, VkCommandPool& cmdPool, const void* data)
	{
		return std::make_shared<VKBuffer>(device, type, size, cmdPool, data);
	}

	VKBuffer::VKBuffer(std::shared_ptr<VKDevice>& device, Type type, VkDeviceSize size, VkCommandPool& cmdPool, const void* data)
		: mDevice(device), mType(type), mSize(size), mCmdPool(cmdPool), mData(data)
	{
		Logger() << "Creting VKBuffer";

		VkBuffer stagingBuffer = VK_NULL_HANDLE;
		VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
		VkMemoryPropertyFlags usageType = {};

		switch (type)
		{
		case Cosmos::VKBuffer::Vertex:
		{
			usageType |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			break;
		}
		
		case Cosmos::VKBuffer::Index:
		{
			usageType |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			break;
		}

		case Cosmos::VKBuffer::Uniform:
		{
			usageType |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
			break;
		}
		}

		// create staging buffer
		CreateBuffer
		(
			VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			&stagingBuffer,
			&stagingBufferMemory
		);

		// map memory area(lData) and copy buffer to it
		void* lData = nullptr;
		vkMapMemory(mDevice->Device(), stagingBufferMemory, 0, mSize, 0, &lData);
		memcpy(lData, mData, (size_t)mSize);
		vkUnmapMemory(mDevice->Device(), stagingBufferMemory);

		// create buffer
		CreateBuffer
		(
			VK_BUFFER_USAGE_TRANSFER_DST_BIT | usageType,
			VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
			&mBuffer,
			&mBufferMemory
		);

		// copies staging to buffer
		CopyBuffer(stagingBuffer, mBuffer);

		vkDestroyBuffer(mDevice->Device(), stagingBuffer, nullptr);
		vkFreeMemory(mDevice->Device(), stagingBufferMemory, nullptr);
	}

	void VKBuffer::Destroy()
	{
		vkDestroyBuffer(mDevice->Device(), mBuffer, nullptr);
		vkFreeMemory(mDevice->Device(), mBufferMemory, nullptr);
	}

	void VKBuffer::CreateBuffer(VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
	{
		// specify buffer
		VkBufferCreateInfo bufferCI = {};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = mSize;
		bufferCI.usage = usage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_ASSERT(vkCreateBuffer(mDevice->Device(), &bufferCI, nullptr, buffer), "Failed to create buffer");

		// alocate memory for specified buffer 
		VkMemoryRequirements memoryReqs = {};
		vkGetBufferMemoryRequirements(mDevice->Device(), *buffer, &memoryReqs);

		VkMemoryAllocateInfo memoryAllocInfo = {};
		memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocInfo.pNext = nullptr;
		memoryAllocInfo.allocationSize = memoryReqs.size;
		memoryAllocInfo.memoryTypeIndex = mDevice->GetMemoryType(memoryReqs.memoryTypeBits, properties);
		VK_ASSERT(vkAllocateMemory(mDevice->Device(), &memoryAllocInfo, nullptr, memory), "Failed to allocate memory for buffer");

		// data is not null, must map the buffer and copy the data
		if (data != nullptr)
		{
			void* mapped;
			VK_ASSERT(vkMapMemory(mDevice->Device(), *memory, 0, mSize, 0, &mapped), "Faled to map memory");
			memcpy(mapped, data, mSize);

			// if host coherency hasn't been requested, do a manual flush to make writes visible
			if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange = {};
				mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = mSize;
				vkFlushMappedMemoryRanges(mDevice->Device(), 1, &mappedRange);
			}

			vkUnmapMemory(mDevice->Device(), *memory);
		}

		// link buffer with allocated memory
		VK_ASSERT(vkBindBufferMemory(mDevice->Device(), *buffer, *memory, 0), "Failed to bind buffer with memory");
	}

	void VKBuffer::CopyBuffer(VkBuffer src, VkBuffer dst)
	{
		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandPool = mCmdPool;
		cmdBufferAllocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(mDevice->Device(), &cmdBufferAllocInfo, &commandBuffer);

		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &beginInfo);

		VkBufferCopy copyRegion = {};
		copyRegion.size = mSize;
		vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);

		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo{};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(mDevice->GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(mDevice->GraphicsQueue());

		vkFreeCommandBuffers(mDevice->Device(), mCmdPool, 1, &commandBuffer);
	}

	VkResult BufferCreate(std::shared_ptr<VKDevice>& device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data)
	{
		// specify buffer
		VkBufferCreateInfo bufferCI = {};
		bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		bufferCI.size = size;
		bufferCI.usage = usage;
		bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		VK_ASSERT(vkCreateBuffer(device->Device(), &bufferCI, nullptr, buffer), "Failed to create buffer");

		// alocate memory for specified buffer 
		VkMemoryRequirements memoryReqs;
		vkGetBufferMemoryRequirements(device->Device(), *buffer, &memoryReqs);

		VkMemoryAllocateInfo memoryAllocInfo = {};
		memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		memoryAllocInfo.pNext = nullptr;
		memoryAllocInfo.allocationSize = memoryReqs.size;
		memoryAllocInfo.memoryTypeIndex = device->GetMemoryType(memoryReqs.memoryTypeBits, properties);
		VK_ASSERT(vkAllocateMemory(device->Device(), &memoryAllocInfo, nullptr, memory), "Failed to allocate memory for buffer");

		// data is not null, must map the buffer and copy the data
		if (data != nullptr)
		{
			void* mapped;
			VK_ASSERT(vkMapMemory(device->Device(), *memory, 0, size, 0, &mapped), "Faled to map memory");
			memcpy(mapped, data, size);

			// if host coherency hasn't been requested, do a manual flush to make writes visible
			if ((properties & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
			{
				VkMappedMemoryRange mappedRange = {};
				mappedRange.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
				mappedRange.memory = *memory;
				mappedRange.offset = 0;
				mappedRange.size = size;
				vkFlushMappedMemoryRanges(device->Device(), 1, &mappedRange);
			}

			vkUnmapMemory(device->Device(), *memory);
		}

		// link buffer with allocated memory
		VK_ASSERT(vkBindBufferMemory(device->Device(), *buffer, *memory, 0), "Failed to bind buffer with memory");

		return VK_SUCCESS;
	}

	VkCommandBuffer BeginSingleTimeCommand(std::shared_ptr<VKDevice>& device, VkCommandPool& commandPool)
	{
		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandPool = commandPool;
		cmdBufferAllocInfo.commandBufferCount = 1;

		VkCommandBuffer commandBuffer;
		vkAllocateCommandBuffers(device->Device(), &cmdBufferAllocInfo, &commandBuffer);

		VkCommandBufferBeginInfo cmdBufferBeginInfo = {};
		cmdBufferBeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		cmdBufferBeginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

		vkBeginCommandBuffer(commandBuffer, &cmdBufferBeginInfo);

		return commandBuffer;
	}

	void EndSingleTimeCommand(std::shared_ptr<VKDevice>& device, VkCommandPool& commandPool, VkCommandBuffer commandBuffer)
	{
		vkEndCommandBuffer(commandBuffer);

		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(device->GraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(device->GraphicsQueue());

		vkFreeCommandBuffers(device->Device(), commandPool, 1, &commandBuffer);
	}

	VkCommandBuffer CreateCommandBuffer(std::shared_ptr<VKDevice>& device, VkCommandPool& cmdPool, VkCommandBufferLevel level, bool begin)
	{
		VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.pNext = nullptr;
		cmdBufferAllocInfo.commandBufferCount = 1;
		cmdBufferAllocInfo.commandPool = cmdPool;
		cmdBufferAllocInfo.level = level;

		VkCommandBuffer cmdBuffer;
		VK_ASSERT(vkAllocateCommandBuffers(device->Device(), &cmdBufferAllocInfo, &cmdBuffer), "Failed to allocate command buffers");

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

	void FlushCommandBuffer(std::shared_ptr<VKDevice>& device, VkCommandPool& cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue, bool free)
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
		VK_ASSERT(vkCreateFence(device->Device(), &fenceCI, nullptr, &fence), "Failed to create fence for command buffer submission");
		VK_ASSERT(vkQueueSubmit(queue, 1, &submitInfo, fence), "Failed to submit command buffer");
		VK_ASSERT(vkWaitForFences(device->Device(), 1, &fence, VK_TRUE, 100000000000), "Failed to wait for fences");

		vkDestroyFence(device->Device(), fence, nullptr);

		if (free)
		{
			vkFreeCommandBuffers(device->Device(), cmdPool, 1, &cmdBuffer);
		}
	}
}