#pragma once

#include "Defines.h"
#include "Renderer/Commander.h"
#include "Renderer/Renderer.h"
#include "Entity/Entity.h"

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
		VKRenderer(std::shared_ptr<Window>& window, Scene* scene);

		// destructor
		virtual ~VKRenderer();

	public:

		// returns the backend instance class object
		virtual std::shared_ptr<Instance> GetInstance() override;

		// returns the backend device class object
		virtual std::shared_ptr<Device> GetDevice() override;

		// returns the backend swapchain class object
		virtual std::shared_ptr<Swapchain> GetSwapchain() override;

	public:

		// returns the vulkan pipeline cache
		virtual VkPipelineCache& PipelineCache() override;

		// returns the pipeline with a given id(name) or nullptr if invalid
		virtual VkPipeline& GetPipeline(std::string nameid) override;

		// returns the descriptor set layout with a given id(name) or nullptr if invalid
		virtual VkDescriptorSetLayout& GetDescriptorSetLayout(std::string nameid) override;

		// returns the pipeline layout with a given id(name) or nullptr if invalid
		virtual VkPipelineLayout& GetPipelineLayout(std::string nameid) override;

		// returns the current in-process frame
		virtual uint32_t CurrentFrame() override;

		// returns the current image index
		virtual uint32_t ImageIndex() override;

		// returns a reference to the commander
		virtual Commander& GetCommander() override;

	public:

		// setup initial resources (called after main renderpass has been created)
		virtual void Intialize() override;

		// updates the renderer
		virtual void OnUpdate() override;

		// links the user interface to the renderer
		virtual void ConnectUI(std::shared_ptr<GUI>& ui) override;

	private:

		// submit all render passes
		virtual void ManageRenderPasses(uint32_t& imageIndex) override;

		// creates global structures shared across the renderer
		virtual void CreateGlobalStates() override;

		// creates custom pipelines
		void CreatePipelines();

	private:

		std::shared_ptr<Window>& mWindow;
		Scene* mScene;
		std::shared_ptr<VKInstance> mInstance;
		std::shared_ptr<VKDevice> mDevice;
		std::shared_ptr<VKSwapchain> mSwapchain;

		Commander mCommander;
		VkPipelineCache mPipelineCache;

		std::vector<VkSemaphore> mImageAvailableSemaphores;
		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;
		uint32_t mCurrentFrame = 0;
		uint32_t mImageIndex = 0;

		std::shared_ptr<GUI> mUI;

		// pipeline objects 
		std::unordered_map<std::string, VkPipeline> mPipelines = {};
		std::unordered_map<std::string, VkDescriptorSetLayout> mDescriptorSetLayouts = {};
		std::unordered_map<std::string, VkPipelineLayout> mPipelineLayouts = {};
	};
}