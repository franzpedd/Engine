#pragma once

#include <vulkan/vulkan.h>
#include <memory>
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
		struct Details
		{
			VkSurfaceCapabilitiesKHR capabilities = {};
			std::vector<VkSurfaceFormatKHR> formats = {};
			std::vector<VkPresentModeKHR> presentModes = {};
		};

	public:

		// returns a smart pointer to a new swapchain class
		static std::shared_ptr<VKSwapchain> Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device);

		// constructor
		VKSwapchain(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device);

		// destructor
		~VKSwapchain();

		// returns the vulkan swapchain
		inline VkSwapchainKHR& Swapchain() { return mSwapchain; }

		// returns the swapchain images
		inline std::vector<VkImage>& Images() { return mImages; }

		// returns the swapchain image views
		inline std::vector<VkImageView> ImageViews() { return mImageViews; }

		// returns the simultaniously rendered images
		inline uint32_t ImageCount() { return mImageCount; }

		// returns swapchain's surface format
		inline VkSurfaceFormatKHR& SurfaceFormat() { return mSurfaceFormat; }

		// returns swapchain's presentation mode
		inline VkPresentModeKHR& PresentMode() { return mPresentMode; }

		// returns swapchain's extent
		inline VkExtent2D& Extent() { return mExtent; }

		// returns the swapchain's color view
		inline VkImageView& ColorView() { return mColorView; }

		// returns the swapchain's depth view
		inline VkImageView& DepthView() { return mDepthView; }

		// returns the sampling in use
		inline VkSampleCountFlagBits MSAA() { return mMSAACount; }

		// returns a reference to the renderer command entry
		inline std::shared_ptr<CommandEntry>& CommandEntries() { return mCommandEntry; }

	public:

		// creates the command pool
		void CreateCommandPool();

		// creates the swapchain commandbuffers
		void CreateCommandBuffers();

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

		// returns the optimal surface format
		VkSurfaceFormatKHR ChooseSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);

		// returns the optimal presentation mode
		VkPresentModeKHR ChoosePresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);

		// returns the optimal swapchain extent
		VkExtent2D ChooseExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	private:

		std::shared_ptr<Window>& mWindow;
		std::shared_ptr<VKInstance>& mInstance;
		std::shared_ptr<VKDevice>& mDevice;

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

		VkSampleCountFlagBits mMSAACount;

		std::shared_ptr<CommandEntry> mCommandEntry;
	};
}