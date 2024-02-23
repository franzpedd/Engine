#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Cosmos
{
	// forward declarations
	class Device;

	// this structure holds all objects shared across all models
	// there should be only one object of this structure, it belongs to the Vulkan renderer
	struct ModelGlobalResource
	{
	public:

		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;
		VkPipeline pipeline = VK_NULL_HANDLE;

	public:

		// creates and populates the objects
		void Initialize(std::shared_ptr<Device> device, VkPipelineCache cache);
	};
}