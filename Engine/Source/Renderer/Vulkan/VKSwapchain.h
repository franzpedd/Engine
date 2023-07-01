#pragma once

#include "Core/Defines.h"
#include "VKInstance.h"
#include "VKDevice.h"

#include <vulkan/vulkan.h>

#include <memory>
#include <vector>

namespace Cosmos
{
	class COSMOS_API VKSwapchain
	{
	public:

		struct SwapchainBuffer
		{
			VkImage Image;
			VkImageView View;
		};

		struct Specification
		{
			VkSurfaceKHR Surface = VK_NULL_HANDLE;
			VkFormat ColorFormat = VK_FORMAT_UNDEFINED;
			VkFormat DepthFormat = VK_FORMAT_UNDEFINED;
			VkColorSpaceKHR ColorSpace = {};
			VkSwapchainKHR Swapchain = VK_NULL_HANDLE;
			u32 ImageCount = 0;
			std::vector<VkImage> Images = {};
			std::vector<SwapchainBuffer> Buffers = {};
			VkExtent2D Extent = {};
			u32 QueueNodeIndex = UINT32_MAX;

			bool VSync = false;
			u32 Width = 0;
			u32 Height = 0;
		};

	public:

		// returns a smart-ptr to a new vkswapchain
		static std::shared_ptr<VKSwapchain> Create(std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, u32 width, u32 height, bool vsync = false);

		// constructor
		VKSwapchain(std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, u32 width, u32 height, bool vsync = false);

		// destructor
		~VKSwapchain();

		// returns the specification
		inline Specification& GetSpecification() { return m_Specification; }

	public:

		// initializes the surface
		void InitSurface();

		// creates the swapchain
		void SetupSwapchain(u32* width, u32* height, bool vSync = false);

		// acquires the next image in the swapchain
		VkResult AcquireNextImage(VkSemaphore presentCompleteSemaphore, u32* imageIndex);

		// queue an image for presentation
		VkResult QueuePresent(VkQueue queue, u32 imageIndex, VkSemaphore waitSemaphore = VK_NULL_HANDLE);

		// destroy used resources for the swapchain
		void Cleanup();

	private:

		std::shared_ptr<VKInstance>& m_Instance;
		std::shared_ptr<VKDevice>& m_Device;
		Specification m_Specification;

	};
}