#pragma once
#include "Defines.h"

#include "Instance.h"
#include "Device.h"
#include "Swapchain.h"

#include <memory>

namespace Cosmos
{
	// forward declaration
	class GUI;
	class Window;
	class Scene;

	class Renderer
	{
	public:

		// returns a smart pointer to a new renderer
		static std::shared_ptr<Renderer> Create();

		// constructor
		Renderer() = default;

		// destructor
		virtual ~Renderer() = default;

	public:

		// returns the backend instance class object
		virtual std::shared_ptr<Instance> GetInstance() = 0;

		// returns the backend device class object
		virtual std::shared_ptr<Device> GetDevice() = 0;

		// returns the backend swapchain class object
		virtual std::shared_ptr<Swapchain> GetSwapchain() = 0;

	public:

		// returns the vulkan pipeline cache
		virtual VkPipelineCache& PipelineCache() = 0;

		// returns the pipeline with a given id(name) or nullptr if invalid
		virtual VkPipeline GetPipeline(std::string nameid) = 0;

		// returns the descriptor set layout with a given id(name) or nullptr if invalid
		virtual VkDescriptorSetLayout GetDescriptorSetLayout(std::string nameid) = 0;

		// returns the pipeline layout with a given id(name) or nullptr if invalid
		virtual VkPipelineLayout GetPipelineLayout(std::string nameid) = 0;

		// returns the current in-process frame
		virtual uint32_t CurrentFrame() = 0;

		// returns the current image index
		virtual uint32_t ImageIndex() = 0;

	public:

		// called after the main loop is finished
		virtual void OnTerminate() = 0;

		// updates the renderer
		virtual void OnUpdate() = 0;

		// creates/recreates global structures shared across the renderer
		virtual void CreateGlobalStates() = 0;
	};
}