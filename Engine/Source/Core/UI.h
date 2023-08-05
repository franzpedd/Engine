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

	public:

		// updates the ui
		void Update();

		// draws the ui
		void Draw(VkCommandBuffer cmd);

		// sets the minimum image count, used whenever the swapchain is resized and image count change
		void SetImageCount(uint32_t count);

	public:

		// ui configuration
		void SetupConfiguration(VkRenderPass& renderPass);

		// ui style
		void SetupCustomStyle();

	private:

		std::shared_ptr<Window>& mWindow;
		std::shared_ptr<VKInstance>& mInstance;
		std::shared_ptr<VKDevice>& mDevice;
		std::shared_ptr<VKSwapchain>& mSwapchain;
		VkCommandPool mCommandPool = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
	};
}