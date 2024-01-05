#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Cosmos
{
	// forward declarations
	class Device;
	class Instance;
	class Window;

	struct CommandEntry;

	class Swapchain
	{
	public:

		struct Details
		{
			VkSurfaceCapabilitiesKHR capabilities = {};
			std::vector<VkSurfaceFormatKHR> formats = {};
			std::vector<VkPresentModeKHR> presentModes = {};
		};

	public:

		// constructor
		Swapchain() = default;

		// destructor
		virtual ~Swapchain() = default;

	public:

		// returns the vulkan swapchain
		virtual VkSwapchainKHR& GetSwapchain() = 0;

		// returns the swapchain images
		virtual std::vector<VkImage>& GetImages() = 0;

		// returns the swapchain image views
		virtual std::vector<VkImageView> GetImageViews() = 0;

		// returns the simultaniously rendered images
		virtual uint32_t GetImageCount() = 0;

		// returns swapchain's surface format
		virtual VkSurfaceFormatKHR& GetSurfaceFormat() = 0;

		// returns swapchain's presentation mode
		virtual VkPresentModeKHR& GetPresentMode() = 0;

		// returns swapchain's extent
		virtual VkExtent2D& GetExtent() = 0;

		// returns the swapchain's color view
		virtual VkImageView& GetColorView() = 0;

		// returns the swapchain's depth view
		virtual VkImageView& GetDepthView() = 0;

	public:

		// creates the swapchain render pass, a render pass containing the backbuffer
		virtual void CreateRenderPass() = 0;

		// creates the swapchain
		virtual void CreateSwapchain() = 0;

		// creates the swapchain image views
		virtual void CreateImageViews() = 0;

		// creates the swapchain framebuffers
		virtual void CreateFramebuffers() = 0;

		// cleans the current swapchain
		virtual void Cleanup() = 0;

		// recreates the swapchain
		virtual void Recreate() = 0;

		// fills all information about the swapchain details
		virtual Details QueryDetails() = 0;
	};
}