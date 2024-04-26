#pragma once

#include "VKDefines.h"
#include "Util/Memory.h"
#include <vector>

namespace Cosmos
{
	// forward declarations
	class VKDevice;
	class VKInstance;
	class VKSurface;
	class Window;

	struct CommandEntry;

	class VKSwapchain
	{
	public:

		struct Details
		{
			VkSurfaceCapabilitiesKHR capabilities = {};
			std::vector<VkSurfaceFormatKHR> formats = {};
			std::vector<VkPresentModeKHR> presentModes = {};
		};

	public:

		// returns a smart pointer to a new swapchain class
		static Shared<VKSwapchain> Create(Shared<VKInstance> instance, Shared<VKDevice> device);

		// constructor
		VKSwapchain(Shared<VKInstance> instance, Shared<VKDevice> device);

		// destructor
		~VKSwapchain();

	public:

		// returns the vulkan swapchain
		VkSwapchainKHR& GetSwapchain();

		// returns the swapchain images
		std::vector<VkImage>& GetImages();

		// returns the swapchain image views
		std::vector<VkImageView> GetImageViews();

		// returns the simultaniously rendered images
		uint32_t GetImageCount();

		// returns swapchain's surface format
		VkSurfaceFormatKHR& GetSurfaceFormat();

		// returns swapchain's presentation mode
		VkPresentModeKHR& GetPresentMode();

		// returns swapchain's extent
		VkExtent2D& GetExtent();

		// returns the swapchain's color view
		VkImageView& GetColorView();

		// returns the swapchain's depth view
		VkImageView& GetDepthView();

	public:

		// creates the swapchain render pass, a render pass containing the backbuffer
		void CreateRenderPass();

		// creates the swapchain
		void CreateSwapchain();

		// creates the swapchain image views
		void CreateImageViews();

		// creates the swapchain framebuffers
		void CreateFramebuffers();

		// cleans the current swapchain
		void Cleanup();

		// recreates the swapchain
		void Recreate();

		// fills all information about the swapchain details
		Details QueryDetails();

	public:

		// creates the swapchain command pool
		void CreateCommandPool();

		// creates teh swapchain command buffers
		void CreateCommandBuffers();

	public:

		// returns the optimal surface format
		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		// returns the optimal presentation mode
		VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		// returns the optimal swapchain extent
		VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	private:

		std::shared_ptr<VKInstance> mInstance;
		std::shared_ptr<VKDevice> mDevice;

		VkSwapchainKHR mSwapchain = VK_NULL_HANDLE;
		std::vector<VkImage> mImages = {};
		std::vector<VkImageView> mImageViews = {};
		uint32_t mImageCount;
		VkSurfaceFormatKHR mSurfaceFormat = {};
		VkPresentModeKHR mPresentMode = {};
		VkExtent2D mExtent = {};

		VkImage mColorImage;
		VkDeviceMemory mColorMemory;
		VkImageView mColorView;
		VkImage mDepthImage;
		VkDeviceMemory mDepthMemory;
		VkImageView mDepthView;
	};
}