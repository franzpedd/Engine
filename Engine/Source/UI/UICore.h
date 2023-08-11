#pragma once

#include "UIElement.h"
#include <vulkan/vulkan.h>
#include <memory>

namespace Cosmos
{
	// forward declarations
	class VKInstance;
	class VKDevice;
	class VKSwapchain;
	class Window;

	struct CommandEntry;

	class UICore
	{
	public:

		// returns a smart pointer to a new user interface
		static std::shared_ptr<UICore> Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain);

		// constructor
		UICore(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain);

		// destructor
		~UICore();

		// returns a reference to all ui elements, used to attach new ui elements
		inline UIElementStack& ElementStack() { return mUIElements; }

		// returns a reference to the renderer command entries
		inline std::shared_ptr<CommandEntry>& CommandEntries() { return mCommanderEntry; }

	public:

		// inits a new frame
		void NewFrame();

		// updates the ui
		void OnUpdate();

		// finishes current frame
		void EndFrame();

		// draws the ui
		void Draw(VkCommandBuffer cmd);

		// sets the minimum image count, used whenever the swapchain is resized and image count change
		void SetImageCount(uint32_t count);

		// handles framebuffer resizes
		void OnResize();

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
		std::shared_ptr<CommandEntry> mCommanderEntry;

		UIElementStack mUIElements;
	};
}