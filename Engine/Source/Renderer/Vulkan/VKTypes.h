#pragma once

#include "Core/Defines.h"
#include "VKBuffer.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Cosmos
{
	// holds buffer of every type of object used in the scene
	struct COSMOS_API UniformBufferSets
	{
		std::shared_ptr<VKBuffer> Scene;
		std::shared_ptr<VKBuffer> Skybox;
		std::shared_ptr<VKBuffer> Params;
	};

	// holds every type of descriptor set used in the scene
	struct COSMOS_API DescriptorSets
	{
		VkDescriptorSet Scene;
		VkDescriptorSet Skybox;
	};

	// used to multisample the image/frame
	struct COSMOS_API MultiSampleTarget
	{
		struct
		{
			VkImage image = VK_NULL_HANDLE;
			VkImageView view = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
		} Color;

		struct
		{
			VkImage image = VK_NULL_HANDLE;
			VkImageView view = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
		} Depth;
	};

	// used to draw only visible areas of the scene
	struct COSMOS_API DepthStencil
	{
		VkImage image = VK_NULL_HANDLE;
		VkDeviceMemory memory = VK_NULL_HANDLE;
		VkImageView view = VK_NULL_HANDLE;
	};

	// used to sincronizate cpu with gpu
	struct COSMOS_API Syncronization
	{
		const u32 RenderAhead = 2;
		std::vector<VkFence> WaitFences;
		std::vector<VkSemaphore> RenderCompleteSemaphores;
		std::vector<VkSemaphore> PresentationCompleteSemaphore;
	};
}