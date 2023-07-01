#pragma once

#include "Core/Defines.h"
#include "VKDevice.h"

#include <vulkan/vulkan.h>

namespace Cosmos
{
	class COSMOS_API VKTexture2D
	{
	public:

		struct Specification
		{
			VkImage image;
			VkImageLayout layout;
			VkDeviceMemory memory;
			VkImageView view;
			VkDescriptorImageInfo descriptorImageInfo;
			VkSampler sampler;
			u32 width;
			u32 height;
			u32 channels;
			u32 mipLevel;
			u32 layerCount;
		};

	public:

		// constructor
		VKTexture2D(std::shared_ptr<VKDevice>& device);

		// destructor
		~VKTexture2D();

		// returns the private members
		inline Specification& GetSpecification() { return m_Specification; }

	public:

		// loads a texture from the file
		void LoadFromFile(std::string path, VkFormat format, VkQueue queue, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		// loads a texture from a buffer
		void LoadFromBuffer(void* buffer, VkDeviceSize size, VkFormat format, u32 width, u32 height, VkQueue queue, VkFilter filter = VK_FILTER_LINEAR, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	private:

		std::shared_ptr<VKDevice>& m_Device;
		Specification m_Specification;
	};

	class VKTextureCubemap
	{
	public:

		struct Specification
		{
			VkImage image;
			VkImageLayout layout;
			VkDeviceMemory memory;
			VkImageView view;
			VkDescriptorImageInfo descriptorImageInfo;
			VkSampler sampler;
			u32 width;
			u32 height;
			u32 channels;
			u32 mipLevel;
			u32 layerCount;
		};

	public:

		// constructor
		VKTextureCubemap(std::shared_ptr<VKDevice>& device);

		// destructor
		~VKTextureCubemap();

		// returns the private members
		inline Specification& GetSpecification() { return m_Specification; }

	public:

		// loads a cubemap from filepath
		void LoadFromFile(std::string path, VkFormat format, VkQueue queue, VkImageUsageFlags usage = VK_IMAGE_USAGE_SAMPLED_BIT, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	private:

		std::shared_ptr<VKDevice>& m_Device;
		Specification m_Specification;
	};
}