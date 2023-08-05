#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <optional>
#include <vector>

namespace Cosmos
{
	// forward declaration
	class VKInstance;
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

	class VKDevice
	{
	public:

		// returns a smart pointer to a new device
		static std::shared_ptr<VKDevice> Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance);

		// constructor
		VKDevice(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance);

		// destructor
		~VKDevice();

		// returns a reference to the vulkan surface
		inline VkSurfaceKHR& Surface() { return mSurface; }

		// returns a reference to the vulkan device
		inline VkDevice& Device() { return mDevice; }

		// returns a reference to the vulkan physical device
		inline VkPhysicalDevice& PhysicalDevice() { return mPhysicalDevice; }

		// returns the graphics queue
		inline VkQueue& GraphicsQueue() { return mGraphicsQueue; }

		// returns the presentation queue
		inline VkQueue& PresentQueue() { return mPresentQueue; }

		// returns the compute queue
		inline VkQueue& ComputeQueue() { return mComputeQueue; }

		// returns the command pool
		inline VkCommandPool& CommandPool() { return mCommandPool; }

		// returns the command buffers
		inline std::vector<VkCommandBuffer>& CommandBuffers() { return mCommandBuffers; }

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

		// creates the command pool
		void CreateCommandPool();

		// creates the command buffers
		void CreateCommandBuffers();

	private:
		
		std::shared_ptr<VKInstance>& mInstance;
		std::shared_ptr<Window>& mWindow;

		VkSurfaceKHR mSurface;
		VkDevice mDevice;
		VkPhysicalDevice mPhysicalDevice;
		VkPhysicalDeviceProperties mProperties;
		VkPhysicalDeviceFeatures mFeatures;
		VkPhysicalDeviceMemoryProperties mMemoryProperties;
		VkQueue mGraphicsQueue;
		VkQueue mPresentQueue;
		VkQueue mComputeQueue;
		VkCommandPool mCommandPool;
		std::vector<VkCommandBuffer> mCommandBuffers;
	};
}