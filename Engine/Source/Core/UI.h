#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Cosmos
{
	// forward declaration
	class VKDevice;
	class VKInstance;
	class VKSwapchain;
	class Window;

	class UI
	{
	public:

		// returns a smart pointer to a new user interface
		static std::shared_ptr<UI> Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain);

		// constructor
		UI(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain);

		// destructor
		~UI();

		// returns a reference to used render pass
		inline VkRenderPass& RenderPass() { return mRenderPass; }

		// returns a reference to used command pool
		inline VkCommandPool& CommandPool() { return mCommandPool; }

		// returns a reference to the command buffers
		inline std::vector<VkCommandBuffer>& CommandBuffers() { return mCommandBuffers; }

		// returns a reference to the frame buffers
		inline std::vector<VkFramebuffer>& Framebuffers() { return mFramebuffers; }

	public:

		// updates the ui
		void Update();

		// draws the ui
		void Draw(VkCommandBuffer cmd);

		// sets the minimum image count, used whenever the swapchain is resized and image count change
		void SetImageCount(uint32_t count);

		// handles framebuffer resizes
		void Resize();

	public:

		// ui configuration
		void SetupConfiguration();

		// create vulkan resources
		void CreateResources();

		// ui style
		void SetupCustomStyle();

	private:

		std::shared_ptr<Window>& mWindow;
		std::shared_ptr<VKInstance>& mInstance;
		std::shared_ptr<VKDevice>& mDevice;
		std::shared_ptr<VKSwapchain>& mSwapchain;

		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;

		// on test
		VkRenderPass mRenderPass = VK_NULL_HANDLE;
		VkCommandPool mCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> mCommandBuffers = {};
		std::vector<VkFramebuffer> mFramebuffers = {};
	};
}