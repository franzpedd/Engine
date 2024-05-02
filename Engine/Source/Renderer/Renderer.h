#pragma once

#include "Defines.h"
#include "Vulkan/VKDefines.h"
#include "Util/Memory.h"

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
		static Shared<Renderer> Create();

		// constructor
		Renderer() = default;

		// destructor
		virtual ~Renderer() = default;

	public:

		// returns the vulkan pipeline cache
		virtual VkPipelineCache GetPipelineCache() = 0;

		// returns the current in-process frame
		virtual uint32_t GetCurrentFrame() = 0;

		// returns the current image index
		virtual uint32_t GetImageIndex() = 0;

	public:

		// updates the renderer
		virtual void OnUpdate() = 0;

		// creates/recreates global structures shared across the renderer
		virtual void CreateGlobalStates() = 0;
	};
}