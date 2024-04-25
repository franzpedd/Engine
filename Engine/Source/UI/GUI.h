#pragma once

#include "Widget.h"
#include "Renderer/Commander.h"
#include "Util/Math.h"

#include "Wrapper_imgui.h"
#include "Wrapper_sdl.h"

#include <memory>

namespace Cosmos
{
	// forward declarations
	class Renderer;
	class Window;

	class GUI
	{
	public:

		struct Fonts
		{
			ImFont* imgui;
			ImFont* vera;
			ImFont* iconFA;
			ImFont* iconLC;
		};

	public:

		// constructor
		GUI(std::shared_ptr<Renderer> renderer);

		// destructor
		~GUI();

		// returns the fonts
		inline Fonts& GetFonts() { return mFonts; }

		// returns a reference to the widgets stack
		inline WidgetStack& Widgets() { return mWidgetStack; }

	public:

		// updates the ui
		void OnUpdate();

		// updates the draw calls with extra draw calls (outside imgui)
		void OnRender();

		// event handling
		void OnEvent(Shared<Event> event);

		// draws the ui
		void Draw(VkCommandBuffer cmd);

		// sets the minimum image count, used whenever the swapchain is resized and image count change
		void SetImageCount(uint32_t count);

	public:

		// ui configuration
		void SetupConfiguration();

		// create vulkan resources
		void CreateResources();

		// sends sdl events to user interface
		static void HandleInternalEvent(SDL_Event* e);

	private:

		std::shared_ptr<Renderer> mRenderer;
		Fonts mFonts;
		WidgetStack mWidgetStack;
	};

	// toogles on or off the mouse cursor
	void UIToggleCursor(bool hide);

	// adds a texture in the user interface for later usage
	VkDescriptorSet AddTexture(VkSampler sampler, VkImageView view, VkImageLayout layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

	// removes a texture in the user interface backend
	void RemoveTexture(VkDescriptorSet descriptor);

	// hidens or unhides the mouse iteraction with mouse and ui
	void ToogleMouseCursor(bool hide);

	// theme custom checkbox
	bool CheckboxEx(const char* label, bool* v);

	// simplified checkbox
	bool CheckboxSimplifiedEx(const char* label, bool* value);

	// slider checkbox
	bool CheckboxSliderEx(const char* label, bool* v);

	// custom vector-3 controls
	bool Vector3Control(const char* label, glm::vec3& values);
}