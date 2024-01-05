#pragma once

#include "Defines.h"
#include "Renderer/Commander.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <optional>
#include <vector>

namespace Cosmos
{
	// forward declaration
	class Instance;
	class Window;

	// main queues used by the vulkan renderer backend
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> graphics;
		std::optional<uint32_t> present;
		std::optional<uint32_t> compute;

		// returns if found all queues
		inline bool IsComplete() { return graphics.has_value() && present.has_value() && compute.has_value(); }
	};

	class Device
	{
	public:

		// constructor
		Device() = default;

		// destructor
		virtual ~Device() = default;

	public:

		// returns a reference to the vulkan surface
		virtual VkSurfaceKHR& GetSurface() = 0;

		// returns a reference to the vulkan device
		virtual VkDevice& GetDevice() = 0;

		// returns a reference to the vulkan physical device
		virtual VkPhysicalDevice& GetPhysicalDevice() = 0;

		// returns a reference to the vulkan physical device features
		virtual VkPhysicalDeviceFeatures& GetFeatures() = 0;

		// returns a reference to the vulkan physical device properties
		virtual VkPhysicalDeviceProperties& GetProperties() = 0;

		// returns the graphics queue
		virtual VkQueue& GetGraphicsQueue() = 0;

		// returns the presentation queue
		virtual VkQueue& GetPresentQueue() = 0;

		// returns the compute queue
		virtual VkQueue& GetComputeQueue() = 0;

		// returns the sampling in use
		virtual VkSampleCountFlagBits GetMSAA() = 0;

		// returns a reference to the renderer command entry
		virtual std::shared_ptr<CommandEntry>& GetMainCommandEntry() = 0;

	public:

		// returns the queue indices for all available queues
		virtual QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) = 0;

		// returns the index of requested type of memory
		virtual uint32_t GetMemoryType(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32* found = nullptr) = 0;

		// returns the maximum MSAA sample the physical device handles
		virtual VkSampleCountFlagBits GetMaxUsableSamples() = 0;
	};
}