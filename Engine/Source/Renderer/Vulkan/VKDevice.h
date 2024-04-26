#pragma once

#include "Defines.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <optional>
#include <vector>

namespace Cosmos
{
	// forward declaration
	class VKInstance;
	class Window;

	class VKDevice
	{
	public:

		struct QueueFamilyIndices
		{
			std::optional<uint32_t> graphics;
			std::optional<uint32_t> present;
			std::optional<uint32_t> compute;

			// returns if found all queues
			inline bool IsComplete() const { return graphics.has_value() && present.has_value() && compute.has_value(); }
		};

	public:

		// returns a smart pointer to a new device
		static std::shared_ptr<VKDevice> Create(std::shared_ptr<VKInstance> instance);

		// constructor
		VKDevice(std::shared_ptr<VKInstance> instance);

		// destructor
		~VKDevice();

		// returns a reference to the vulkan surface
		VkSurfaceKHR& GetSurface();

		// returns a reference to the vulkan device
		VkDevice& GetDevice();

		// returns a reference to the vulkan physical device
		VkPhysicalDevice& GetPhysicalDevice();

		// returns a reference to the vulkan physical device features
		VkPhysicalDeviceFeatures& GetFeatures();

		// returns a reference to the vulkan physical device properties
		VkPhysicalDeviceProperties& GetProperties();

		// returns the graphics queue
		VkQueue& GetGraphicsQueue();

		// returns the presentation queue
		VkQueue& GetPresentQueue();

		// returns the compute queue
		VkQueue& GetComputeQueue();

		// returns the sampling in use
		VkSampleCountFlagBits GetMSAA();

	public:

		// returns the queue indices for all available queues
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface);

		// returns the index of requested type of memory
		uint32_t GetMemoryType(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32* found = nullptr);

		// returns the maximum MSAA sample the physical device handles
		VkSampleCountFlagBits GetMaxUsableSamples();

	private:

		// selects the most suitable physical device available
		void SelectPhysicalDevice();

		// creates a logical device out of the choosen physical device
		void CreateLogicalDevice();

	private:

		std::shared_ptr<VKInstance> mInstance;

		VkSurfaceKHR mSurface;
		VkDevice mDevice;
		VkPhysicalDevice mPhysicalDevice;
		VkPhysicalDeviceProperties mProperties;
		VkPhysicalDeviceFeatures mFeatures;
		VkPhysicalDeviceMemoryProperties mMemoryProperties;
		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;
		VkQueue mComputeQueue;
		VkSampleCountFlagBits mMSAACount;
	};
}