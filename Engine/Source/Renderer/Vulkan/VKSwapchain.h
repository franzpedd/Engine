#pragma once

#include "Renderer/Swapchain.h"
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

	class VKSwapchain : public Swapchain
	{
	public:

		// returns a smart pointer to a new swapchain class
		static std::shared_ptr<VKSwapchain> Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device);

		// constructor
		VKSwapchain(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device);

		// destructor
		virtual ~VKSwapchain();

	public:

		// returns the vulkan swapchain
		virtual VkSwapchainKHR& GetSwapchain() override;

		// returns the swapchain images
		virtual std::vector<VkImage>& GetImages() override;

		// returns the swapchain image views
		virtual std::vector<VkImageView> GetImageViews() override;

		// returns the simultaniously rendered images
		virtual uint32_t GetImageCount() override;

		// returns swapchain's surface format
		virtual VkSurfaceFormatKHR& GetSurfaceFormat() override;

		// returns swapchain's presentation mode
		virtual VkPresentModeKHR& GetPresentMode() override;

		// returns swapchain's extent
		virtual VkExtent2D& GetExtent() override;

		// returns the swapchain's color view
		virtual VkImageView& GetColorView() override;

		// returns the swapchain's depth view
		virtual VkImageView& GetDepthView() override;

	public:

		// creates the swapchain render pass, a render pass containing the backbuffer
		virtual void CreateRenderPass() override;

		// creates the swapchain
		virtual void CreateSwapchain() override;

		// creates the swapchain image views
		virtual void CreateImageViews() override;

		// creates the swapchain framebuffers
		virtual void CreateFramebuffers() override;

		// cleans the current swapchain
		virtual void Cleanup() override;

		// recreates the swapchain
		virtual void Recreate() override;

		// fills all information about the swapchain details
		virtual Details QueryDetails() override;

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
	};
}