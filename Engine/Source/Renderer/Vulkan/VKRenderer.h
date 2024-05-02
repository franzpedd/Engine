#pragma once

#include "Defines.h"
#include "VKDefines.h"

#include "Renderer/Renderer.h"
#include "Entity/Entity.h"

#include "VKBuffer.h"
#include "VKCommander.h"
#include "VKInstance.h"
#include "VKDevice.h"
#include "VKPipeline.h"
#include "VKSwapchain.h"

#include "Util/Memory.h"

namespace Cosmos
{
	// forward declaration
	class GUI;
	class Window;
	class Scene;

	class VKRenderer : public Renderer
	{
	public:

		// constructor
		VKRenderer();

		// destructor
		virtual ~VKRenderer();

	public:

		// returns the backend instance class object
		inline Shared<VKInstance> GetInstance() { return mInstance; }

		// returns the backend device class object
		inline Shared<VKDevice> GetDevice() { return mDevice; };

		// returns the backend swapchain class object
		inline Shared<VKSwapchain> GetSwapchain() { return mSwapchain; }

		// returns a reference to the pipelines
        inline std::unordered_map<std::string, Shared<VKPipeline>>& GetPipelinesRef() { return mPipelines; }

	public:

		// returns the vulkan pipeline cache
		inline virtual VkPipelineCache GetPipelineCache() override { return mPipelineCache; }

		// returns the current in-process frame
		inline virtual uint32_t GetCurrentFrame() override { return mCurrentFrame; }

		// returns the current image index
		inline virtual uint32_t GetImageIndex() override { return mImageIndex; }

	public:

		// updates the renderer
		virtual void OnUpdate() override;

		// creates global structures shared across the renderer
		virtual void CreateGlobalStates() override;

	private:

		// submit all render passes
		void ManageRenderPasses(uint32_t& imageIndex);

		// creates renderer resources
		void CreateResources();

	private:

		Shared<VKInstance> mInstance;
		Shared<VKDevice> mDevice;
		Shared<VKSwapchain> mSwapchain;

		Shared<VKCommander> mCommander;
		VkPipelineCache mPipelineCache;
		std::unordered_map<std::string, Shared<VKPipeline>> mPipelines = {};

		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;
		uint32_t mCurrentFrame = 0;
		uint32_t mImageIndex = 0;
	};
}