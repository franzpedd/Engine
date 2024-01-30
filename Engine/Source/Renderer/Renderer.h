#pragma once

#include "Buffer.h"
#include "Defines.h"
#include "Commander.h"
#include "Entity/Entity.h"

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
		static std::shared_ptr<Renderer> Create(std::shared_ptr<Window>& window, Scene* scene);

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
		virtual VkPipeline GetPipeline(std::string name) = 0;

		// returns the current in-process frame
		virtual uint32_t CurrentFrame() = 0;

		// returns the current image index
		virtual uint32_t ImageIndex() = 0;

		// returns a reference to the commander
		virtual Commander& GetCommander() = 0;

	public:

		// updates the renderer
		virtual void OnUpdate() = 0;

		// links the user interface to the renderer
		virtual void ConnectUI(std::shared_ptr<GUI>& ui) = 0;

	private:

		// submit all render passes
		virtual void ManageRenderPasses(uint32_t& imageIndex) = 0;

		// creates global structures shared across the renderer
		virtual void CreateGlobalStates() = 0;
	};
}