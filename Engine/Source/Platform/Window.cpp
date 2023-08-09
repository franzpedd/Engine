#include "Window.h"

#include "Util/Logger.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <imgui.h>

namespace Cosmos
{
	std::shared_ptr<Window> Window::Create(const char* title, int width, int height)
	{
		return std::make_shared<Window>(title, width, height);
	}

	Window::Window(const char* title, int width, int height)
	{
		mTitle = title;
		mWidth = width;
		mHeight = height;

		LOG_ASSERT(glfwInit() == 1, "Failed to initialize GLFW");
		LOG_ASSERT(glfwVulkanSupported() == 1, "Your computer doesnt minimally support Vulkan");

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

		mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
		LOG_ASSERT(mWindow != nullptr, "Failed to create Window");

		glfwSetWindowUserPointer(mWindow, reinterpret_cast<void*>(this));
		SetCallbacks();
	}

	Window::~Window()
	{
		glfwDestroyWindow(mWindow);
		glfwTerminate();
	}

	const char* Window::GetTitle()
	{
		return mTitle;
	}

	void Window::SetTitle(const char* title)
	{
		glfwSetWindowTitle(mWindow, title);
		mTitle = title;
	}

	int Window::GetWidth()
	{
		return mWidth;
	}

	void Window::SetWidth(int width)
	{
		glfwSetWindowSize(mWindow, width, mHeight);
		mWidth = width;
	}

	int Window::GetHeight()
	{
		return mHeight;
	}

	void Window::SetHeight(int height)
	{
		glfwSetWindowSize(mWindow, mWidth, height);
		mHeight = height;
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
	}

	bool Window::ShouldQuit()
	{
		return glfwWindowShouldClose(mWindow);
	}

	bool Window::ShouldResizeWindow()
	{
		return mShouldResizeWindow;
	}

	void Window::HintResizeWindow(bool value)
	{
		mShouldResizeWindow = value;
	}

	int Window::CreateWindowSurface(void* instance, void* surface, const void* allocator)
	{
		return glfwCreateWindowSurface(reinterpret_cast<VkInstance>(instance), mWindow, reinterpret_cast<const VkAllocationCallbacks*>(allocator), reinterpret_cast<VkSurfaceKHR*>(surface));
	}

	void Window::GetFramebufferSize(int* width, int* height)
	{
		glfwGetFramebufferSize(mWindow, width, height);
	}

	void Window::WaitEvents()
	{
		glfwWaitEvents();
	}

	const char** Window::GetRequiredInstanceExtensions(unsigned int* count)
	{
		return glfwGetRequiredInstanceExtensions(count);
	}

	void Window::SetCallbacks()
	{
		// internal error
		glfwSetErrorCallback([](int error, const char* desc)
			{
				LOG_TO_TERMINAL(Logger::Severity::Error, "GLFW Error %d:%s", error, desc);
			});

		// window resized
		glfwSetFramebufferSizeCallback(mWindow, [](GLFWwindow* window, int width, int height)
			{
				Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
				win->HintResizeWindow(true);
			});

		// unicode key presed
		glfwSetCharCallback(mWindow, [](GLFWwindow* window, unsigned int keycode)
			{
				Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			});

		// keyboard key pressed
		glfwSetKeyCallback(mWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			});

		// mouse button pressed
		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods)
			{
				Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			});

		// mouse scrolled
		glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double x, double y)
			{
				Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			});

		// mouse moved
		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double x, double y)
			{
				Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			});
	}
}