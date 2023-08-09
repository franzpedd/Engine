#pragma once

#include "Commander.h"

#include "Vulkan/VKInstance.h"
#include "Vulkan/VKDevice.h"
#include "Vulkan/VKDrawable.h"
#include "Vulkan/VKSwapchain.h"
#include "Vulkan/VKUtility.h"

#include <vulkan/vulkan.h>
#include <memory>

namespace Cosmos
{
	// forward declaration
	class UI;
	class Window;

	class Renderer
	{
	public:

		// returns a smart poitner to a new renderer
		static std::shared_ptr<Renderer> Create(std::shared_ptr<Window>& window);

		// constructor
		Renderer(std::shared_ptr<Window>& window);

		// destructor
		~Renderer();

	public:

		// returns the backend instance class object
		inline std::shared_ptr<VKInstance>& BackendInstance() { return mInstance; }

		// returns the backend device class object
		inline std::shared_ptr<VKDevice>& BackendDevice() { return mDevice; }

		// returns the backend swapchain class object
		inline std::shared_ptr<VKSwapchain>& BackendSwapchain() { return mSwapchain; }

		// returns a reference to the renderer commander
		inline std::shared_ptr<Commander>& RendererCommander() { return mCommander; }

	public:

		// returns the user interface
		inline std::shared_ptr<UI>& UserInterface() { return mUI; }

		// returns the vulkan render pass
		inline VkRenderPass& RenderPass() { return mRenderPass; }

		// returns the vulkan pipeline cache
		inline VkPipelineCache& PipelineCache() { return mPipelineCache; }

		// returns the current in-process frame
		inline uint32_t CurrentFrame() { return mCurrentFrame; }

		// returns the current image index
		inline uint32_t ImageIndex() { return mImageIndex; }

		// returns the globally used msaa factor
		inline VkSampleCountFlagBits MSAA() { return mMSAACount; }

	public:

		// updates the renderer
		void OnUpdate();

	private:

		// renderer render backend
		void Render();

		// submit all render passes
		void ManageRenderPasses(uint32_t& imageIndex);

		// creates global structures shared across the renderer
		void CreateGlobalStates();

	private:

		std::shared_ptr<Window>& mWindow;
		std::shared_ptr<VKInstance> mInstance;
		std::shared_ptr<VKDevice> mDevice;
		std::shared_ptr<VKSwapchain> mSwapchain;

		std::shared_ptr<Commander> mCommander;

		std::shared_ptr<UI> mUI;

		VkPipelineCache mPipelineCache;
		VkRenderPass mRenderPass;
		VkSampleCountFlagBits mMSAACount;

		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;
		uint32_t mCurrentFrame = 0;
		uint32_t mImageIndex = 0;
	};
}