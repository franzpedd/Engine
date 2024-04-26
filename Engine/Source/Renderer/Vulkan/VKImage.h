#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Cosmos
{
	// forward declarations
	class VKDevice;

	// creates an image
	void CreateImage(std::shared_ptr<VKDevice> device, uint32_t width, uint32_t height, uint32_t mipLevels, VkSampleCountFlagBits samples, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& memory);

	// returns a sampler
	VkSampler CreateSampler(std::shared_ptr<VKDevice> device, VkFilter min, VkFilter mag, VkSamplerAddressMode u, VkSamplerAddressMode v, VkSamplerAddressMode w, float mipLevels = 1.0f);

	// modifies an image layout to a new one
	void TransitionImageLayout(std::shared_ptr<VKDevice> device, VkCommandPool& cmdPool, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels = 1);

	// creates an image view
	VkImageView CreateImageView(std::shared_ptr<VKDevice> device, VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevel = 1);

	// creates an image memory barrier
	void InsertImageMemoryBarrier(VkCommandBuffer cmdbuffer, VkImage image, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, VkImageLayout oldImageLayout, VkImageLayout newImageLayout, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, VkImageSubresourceRange subresourceRange);

	// returns a optimal format given the specification
	VkFormat FindSuitableFormat(std::shared_ptr<VKDevice> device, const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);

	// returns the optimal depth format
	VkFormat FindDepthFormat(std::shared_ptr<VKDevice> device);
}