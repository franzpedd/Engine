#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

// how many frames are simultaneosly rendered
#define MAX_FRAMES_IN_FLIGHT 2

namespace Cosmos
{
	// forward declaration
	class VKDevice;

	// creates an image
	void CreateImage(std::shared_ptr<VKDevice>& device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory);

	// creates an image view
	VkImageView CreateImageView(std::shared_ptr<VKDevice>& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags);
	// creates a buffer on the gpu
	void BufferCreate(std::shared_ptr<VKDevice>& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	// returns a optimal format given the specification
	VkFormat FindSuitableFormat(std::shared_ptr<VKDevice>& device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	// returns the optimal depth format
	VkFormat FindDepthFormat(std::shared_ptr<VKDevice>& device);

	// starts the recording of a once-used command buffer
	VkCommandBuffer BeginSingleTimeCommand(std::shared_ptr<VKDevice>& device, VkCommandPool& commandPool);

	// ends the recording of a once-used command buffer
	void EndSingleTimeCommand(std::shared_ptr<VKDevice>& device, VkCommandPool& commandPool, VkCommandBuffer commandBuffer);

	// creates an image memory barrier
	void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

	//
}