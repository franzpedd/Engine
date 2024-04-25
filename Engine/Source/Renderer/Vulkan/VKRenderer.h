#pragma once

#include "Defines.h"
#include "Renderer/Commander.h"
#include "Renderer/Renderer.h"
#include "Entity/Entity.h"

#include "GlobalResources.h"
#include "VKBuffer.h"
#include "VKInstance.h"
#include "VKDevice.h"
#include "VKSwapchain.h"

#include <vulkan/vulkan.h>
#include <memory>

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
		Shared<VKInstance> GetInstance();

		// returns the backend device class object
		virtual std::shared_ptr<Device> GetDevice() override;

		// returns the backend swapchain class object
		virtual std::shared_ptr<Swapchain> GetSwapchain() override;

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

		// called after the main loop is finished
		virtual void OnTerminate() override;

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
		std::shared_ptr<VKDevice> mDevice;
		std::shared_ptr<VKSwapchain> mSwapchain;

		Commander* mCommander;
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