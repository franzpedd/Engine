#pragma once

#include "Util/Keycodes.h"
#include <memory>

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

		// returns a smart pointer to a new window
		static std::shared_ptr<Window> Create(const char* title, int width, int height);

		// constructor
		Window(const char* title, int width, int height);

		// destructor
		~Window();

		// returns the native glfw window
		inline GLFWwindow* NativeWindow() { return mWindow; }

	public:

		// returns private members
		inline Data& GetData() { return mData; }

		// returns current time
		double GetTime();

		// return window's title
		const char* GetTitle();

		// sets a new window title
		void SetTitle(const char* title);

		// return window's width
		int GetWidth();

		// sets a new window width
		void SetWidth(int width);

		// returns window's height
		int GetHeight();

		// sets a new window height
		void SetHeight(int height);

		// returns if a key is pressed
		bool IsKeyDown(Keycode key);

		// returns if a button is pressed
		bool IsButtonDown(Buttoncode button);

	public:

		// updates the window
		void OnUpdate();

		// checks if application quit was requested
		bool ShouldQuit();

		// checks if window resize was requested
		bool ShouldResizeWindow();

		// sets the window should resize flag
		void HintResizeWindow(bool value);

		// hides/unhides cursor
		void ToogleCursorMode(bool hide);

		// creates a window surface for vulkan
		int CreateWindowSurface(void* instance, void* surface, const void* allocator);

		// gets the framebuffer size
		void GetFramebufferSize(int* width, int* height);

		// waits for all window events to be handled
		void WaitEvents();

	public:

		// returns VK_KHR_surface and OS-specific extension
		static const char** GetRequiredInstanceExtensions(unsigned int* count);

	private:

		// set event callbacks
		void SetCallbacks();

	private:

		const char* mTitle;
		int mWidth;
		int mHeight;
		GLFWwindow* mWindow;
		bool mShouldResizeWindow = false;
		Data mData;
	};
}