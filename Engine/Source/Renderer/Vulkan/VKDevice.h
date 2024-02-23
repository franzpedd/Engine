#pragma once

#include "Defines.h"
#include "Renderer/Commander.h"
#include "Renderer/Device.h"
#include <vulkan/vulkan.h>
#include <memory>
#include <optional>
#include <vector>

namespace Cosmos
{
	// forward declaration
	class VKInstance;
	class Window;

	class VKDevice : public Device
	{
	public:

		// returns a smart pointer to a new device
		static std::shared_ptr<VKDevice> Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance> instance);

		// constructor
		VKDevice(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance> instance);

		// destructor
		virtual ~VKDevice();

		// returns a reference to the vulkan surface
		virtual VkSurfaceKHR& GetSurface() override;

		// returns a reference to the vulkan device
		virtual VkDevice& GetDevice() override;

		// returns a reference to the vulkan physical device
		virtual VkPhysicalDevice& GetPhysicalDevice() override;

		// returns a reference to the vulkan physical device features
		virtual VkPhysicalDeviceFeatures& GetFeatures() override;

		// returns a reference to the vulkan physical device properties
		virtual VkPhysicalDeviceProperties& GetProperties() override;

		// returns the graphics queue
		virtual VkQueue& GetGraphicsQueue() override;

		// returns the presentation queue
		virtual VkQueue& GetPresentQueue() override;

		// returns the compute queue
		virtual VkQueue& GetComputeQueue() override;

		// returns the sampling in use
		virtual VkSampleCountFlagBits GetMSAA() override;

	public:

		// returns the queue indices for all available queues
		virtual QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) override;

		// returns the index of requested type of memory
		virtual uint32_t GetMemoryType(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32* found = nullptr) override;

		// returns the maximum MSAA sample the physical device handles
		virtual VkSampleCountFlagBits GetMaxUsableSamples() override;

	private:

		// selects the most suitable physical device available
		void SelectPhysicalDevice();

		// creates a logical device out of the choosen physical device
		void CreateLogicalDevice();

	private:

		std::shared_ptr<VKInstance> mInstance;
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
		VkSampleCountFlagBits mMSAACount;
	};
}