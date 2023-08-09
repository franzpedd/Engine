#pragma once

#include "Util/Math.h"
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

	struct CommandEntry;

	class UIElement
	{
	public:

		// constructor
		UIElement() = default;

		// destructor
		virtual ~UIElement() = default;

	public:

		// call to handle initial resources before start updating
		virtual void OnCreation() = 0;

		// call to handle final operations on the element
		virtual void OnDeletion() = 0;

		// updates the ui element
		virtual void OnUpdate() = 0;

		// window was recently resized
		virtual void OnResize() = 0;
	};

	// this class is a vector wrapper to manage multiple ui elements
	class UIElementStack
	{
	public:

		// constructor
		UIElementStack();

		// destructor
		~UIElementStack();

		// returns a reference to the elements vector
		std::vector<UIElement*>& Elements() { return mElements; }

	public:

		// pushes an ui to the top half of the stack
		void PushOver(UIElement* element);

		// pops an ui element from the top half of the stack
		void PopOver(UIElement* element);

		// pushes an ui to the bottom half of the stack
		void Push(UIElement* element);

		// pops an ui element from the bottom half of the stack
		void Pop(UIElement* element);

	public:

		// iterators
		std::vector<UIElement*>::iterator begin() { return mElements.begin(); }
		std::vector<UIElement*>::iterator end() { return mElements.end(); }
		std::vector<UIElement*>::reverse_iterator rbegin() { return mElements.rbegin(); }
		std::vector<UIElement*>::reverse_iterator rend() { return mElements.rend(); }
		std::vector<UIElement*>::const_iterator begin() const { return mElements.begin(); }
		std::vector<UIElement*>::const_iterator end()	const { return mElements.end(); }
		std::vector<UIElement*>::const_reverse_iterator rbegin() const { return mElements.rbegin(); }
		std::vector<UIElement*>::const_reverse_iterator rend() const { return mElements.rend(); }

	private:

		std::vector<UIElement*> mElements;
		uint32_t mMiddlePos = 0;
	};

	// interface to an UI object, all ui elements are inherited from this class
	class UI
	{
	public:

		// returns a smart pointer to a new user interface
		static std::shared_ptr<UI> Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain);

		// constructor
		UI(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain);

		// destructor
		~UI();

		// returns a reference to all ui elements, used to attach new ui elements
		inline UIElementStack& ElementStack() { return mUIElements; }

		// returns a reference to used render pass
		inline VkRenderPass& RenderPass() { return mRenderPass; }

		// returns a reference to used command pool
		inline VkCommandPool& CommandPool() { return mCommandPool; }

		// returns a reference to the command buffers
		inline std::vector<VkCommandBuffer>& CommandBuffers() { return mCommandBuffers; }

		// returns a reference to the frame buffers
		inline std::vector<VkFramebuffer>& Framebuffers() { return mFramebuffers; }

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

		UIElementStack mUIElements;

		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		VkRenderPass mRenderPass = VK_NULL_HANDLE;
		VkCommandPool mCommandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> mCommandBuffers = {};
		std::vector<VkFramebuffer> mFramebuffers = {};

		// on test
		std::shared_ptr<CommandEntry> mCommanderEntry;
	};
}

// interface to imgui functions
// currently using this to avoid including imgui as a dependecy to other projects
// wrapping this as I go
namespace Cosmos::ui
{
	// starts the context for new window
	void Begin(const char* title);

	// ends the context for last bound window
	void End();

	// adds a texture in the user interface for later usage
	VkDescriptorSet AddTexture(VkSampler sampler, VkImageView view, VkImageLayout layout);

	// removes a texture from the user interface pool
	void RemoveTexture(VkDescriptorSet descriptorSet);

	// shows an image to the bound ui window
	void Image(VkDescriptorSet image, Vec2& size);

	// returns the window size of currently bound window
	Vec2 GetContentRegionAvail();
}