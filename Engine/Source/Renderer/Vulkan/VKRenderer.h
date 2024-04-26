#pragma once

#include "Defines.h"
#include "VKDefines.h"

#include "Renderer/Renderer.h"
#include "Entity/Entity.h"

#include "GlobalResources.h"
#include "VKBuffer.h"
#include "VKCommander.h"
#include "VKInstance.h"
#include "VKDevice.h"
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

	public:

		// returns the vulkan pipeline cache
		virtual VkPipelineCache& PipelineCache() override;

		// returns the pipeline with a given id(name) or nullptr if invalid
		virtual VkPipeline GetPipeline(std::string nameid) override;

		// returns the descriptor set layout with a given id(name) or nullptr if invalid
		virtual VkDescriptorSetLayout GetDescriptorSetLayout(std::string nameid) override;

		// returns the pipeline layout with a given id(name) or nullptr if invalid
		virtual VkPipelineLayout GetPipelineLayout(std::string nameid) override;

		// returns the current in-process frame
		virtual uint32_t CurrentFrame() override;

		// returns the current image index
		virtual uint32_t ImageIndex() override;

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

		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;
		uint32_t mCurrentFrame = 0;
		uint32_t mImageIndex = 0;

		// pipeline objects 
		std::unordered_map<std::string, VkPipeline> mPipelines = {};
		std::unordered_map<std::string, VkDescriptorSetLayout> mDescriptorSetLayouts = {};
		std::unordered_map<std::string, VkPipelineLayout> mPipelineLayouts = {};

		// model global resource
		ModelGlobalResource mModelGlobalResource;
	};
}