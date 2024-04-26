#pragma once

#include "Renderer/Buffer.h"
#include <vulkan/vulkan.h>

namespace Cosmos
{
	// forward declarations
	class VKDevice;

	// still used in texture and model class, when testing remove this and rework VKBuffer to support staging only when required
	// creates a buffer on the gpu, used for buffers without staging (as VKBuffer class uses them)
	VkResult BufferCreate(std::shared_ptr<VKDevice> device, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* memory, void* data = nullptr);

	// command buffer will be handled with a class, using functions at the momment
	// starts the recording of a once-used command buffer
	VkCommandBuffer BeginSingleTimeCommand(std::shared_ptr<VKDevice> device, VkCommandPool& commandPool);

	// ends the recording of a once-used command buffer
	void EndSingleTimeCommand(std::shared_ptr<VKDevice> device, VkCommandPool& commandPool, VkCommandBuffer commandBuffer);

	// creates a command buffer given a command pool (usefull while using swapchain's command buffer)
	VkCommandBuffer CreateCommandBuffer(std::shared_ptr<VKDevice> device, VkCommandPool& cmdPool, VkCommandBufferLevel level, bool begin = false);

	// initializes the command buffer
	void BeginCommandBuffer(VkCommandBuffer cmdBuffer);

	// finishes the recording of a command buffer and send it to the queue
	void FlushCommandBuffer(std::shared_ptr<VKDevice> device, VkCommandPool& cmdPool, VkCommandBuffer cmdBuffer, VkQueue queue, bool free = false);
}