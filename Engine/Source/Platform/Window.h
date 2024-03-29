#pragma once

#include "Platform/Keycodes.h"
#include <memory>
#include <vulkan/vulkan.h>

// forward declaration
struct GLFWwindow;

namespace Cosmos
{
	class Window
	{
	public:

		struct Data
		{
			bool lockMousePos = false;
			bool mouseFirstMoved = true;
			float mouseLastX = 0.0f;
			float mouseLastY = 0.0f;
		};

	public:

		// constructor
		Window(const char* title, int width, int height);

		// destructor
		~Window();

		// returns the singleton
		static inline Window* Get() { return sWindow; }

		// returns the native glfw window
		inline GLFWwindow* GetNativeWindow() { return mWindow; }

		// returns private members
		inline Data& GetData() { return mData; }

		// returns current time
		double GetTime();

	public:

		// returns the cursor position
		void GetCursorPosition(double* x, double* y);

		// return window's title
		const char* GetTitle() const;

		// sets a new window title
		void SetTitle(const char* title);

		// return window's width
		int GetWidth() const;

		// sets a new window width
		void SetWidth(int width);

		// returns window's height
		int GetHeight() const;

		// sets a new window height
		void SetHeight(int height);

		// returns if a key is pressed
		bool IsKeyDown(Keycode key);

		// returns if a button is pressed
		bool IsButtonDown(Buttoncode button);

		// returns if main window object is focused
		int Hovered();

	public:

		// updates the window
		void OnUpdate();

		// checks if application quit was requested
		bool ShouldQuit();

		// checks if window resize was requested
		bool ShouldResizeWindow() const;

		// sets the window should resize flag
		void HintResizeWindow(bool value);

		// hides/unhides cursor
		void ToogleCursorMode(bool hide);

		// creates a window surface for vulkan
		int CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface, const VkAllocationCallbacks* allocator);

		// gets the framebuffer size
		void GetFramebufferSize(int* width, int* height);

		// returns the window's aspect ratio
		float GetAspectRatio();

		// waits for all window events to be handled
		void WaitEvents();

	public:

		// returns VK_KHR_surface and OS-specific extension
		static const char** GetRequiredInstanceExtensions(unsigned int* count);

	private:

		// set event callbacks
		void SetCallbacks();

	private:

		static Window* sWindow;
		const char* mTitle;
		int mWidth;
		int mHeight;
		GLFWwindow* mWindow;
		bool mShouldResizeWindow = false;
		Data mData;
	};
}