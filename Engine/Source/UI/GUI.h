#pragma once

#include "Entity/Entity.h"
#include <vulkan/vulkan.h>

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26451 )
#endif

#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <imguizmo.h>

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#include <memory>

namespace Cosmos
{
	// forward declarations
	class Renderer;
	class Window;

	struct CommandEntry;

	class GUI
	{
	public:

		struct Fonts
		{
			ImFont* imgui;
			ImFont* vera;
			ImFont* icons;
		};

	public:

		// returns a smart pointer to a new user interface
		static std::shared_ptr<GUI> Create(std::shared_ptr<Window>& window, std::shared_ptr<Renderer>& renderer);

		// constructor
		GUI(std::shared_ptr<Window>& window, std::shared_ptr<Renderer>& renderer);

		// destructor
		~GUI();

		// returns a reference to the renderer command entries
		inline std::shared_ptr<CommandEntry>& CommandEntries() { return mCommandEntry; }

		// returns the fonts
		inline Fonts& GetFonts() { return mFonts; }

	public:

		// updates the ui
		void OnUpdate(EntityStack& entities);

		// draws the ui
		void Draw(VkCommandBuffer cmd);

		// sets the minimum image count, used whenever the swapchain is resized and image count change
		void SetImageCount(uint32_t count);

		// handles framebuffer resizes
		void OnWindowResize(EntityStack& entities);

	public:

		// ui configuration
		void SetupConfiguration();

		// create vulkan resources
		void CreateResources();

		// ui style
		void SetupCustomStyle();

	private:

		std::shared_ptr<Window>& mWindow;
		std::shared_ptr<Renderer>& mRenderer;
		std::shared_ptr<CommandEntry> mCommandEntry;

		Fonts mFonts;
	};

	// adds a texture in the user interface for later usage
	VkDescriptorSet AddTexture(VkSampler sampler, VkImageView view, VkImageLayout layout);

	// hidens or unhides the mouse iteraction with mouse and ui
	void ToogleMouseCursor(bool hide);

	// theme custom checkbox
	bool CheckboxEx(const char* label, bool* v);

	// simplified checkbox
	bool CheckboxSimplifiedEx(const char* label, bool* value);

	// slider checkbox
	bool CheckboxSliderEx(const char* label, bool* v);
}