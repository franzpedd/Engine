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
	VkImageView CreateImageView(std::shared_ptr<VKDevice>& device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);

	// perform an image layout modification of a given image
	void TransitionImageLayout(std::shared_ptr<VKDevice>& device, VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);

	// creates a buffer on the gpu
	void BufferCreate(std::shared_ptr<VKDevice>& device, VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);

	// copies a buffer from the gpu to another location on it
	void BufferCopy(std::shared_ptr<VKDevice>& device, VkBuffer src, VkBuffer dst, VkDeviceSize size);

	// copies a buffer to an image
	void BufferCopyToImage(std::shared_ptr<VKDevice>& device, VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);

	// generates image mipmaps
	void GenerateMipmaps(std::shared_ptr<VKDevice>& device, VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);

	// returns a optimal format given the specification
	VkFormat FindSuitableFormat(std::shared_ptr<VKDevice>& device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	// returns the optimal depth format
	VkFormat FindDepthFormat(std::shared_ptr<VKDevice>& device);

	// starts the recording of a once-used command buffer
	VkCommandBuffer BeginSingleTimeCommand(std::shared_ptr<VKDevice>& device);

	// ends the recording of a once-used command buffer
	void EndSingleTimeCommand(std::shared_ptr<VKDevice>& device, VkCommandBuffer commandBuffer);
}