#include "epch.h"
#include "Window.h"

#include "Core/Application.h"

#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <glfw/glfw3.h>
#include <imgui.h>
#include <backends/imgui_impl_glfw.h>

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

		if (glfwInit() != GLFW_TRUE)
		{
			LOG_ASSERT(false, "Failed to initialize GLFW");
		}
		
		if (glfwVulkanSupported() != GLFW_TRUE)
		{
			LOG_ASSERT(false, "Your computer doesnt minimally support Vulkan");
		}
		
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

	double Window::GetTime()
	{
		return glfwGetTime();
	}

	void Window::GetCursorPosition(double* x, double* y)
	{
		glfwGetCursorPos(mWindow, x, y);
	}

	const char* Window::GetTitle() const
	{
		return mTitle;
	}

	void Window::SetTitle(const char* title)
	{
		glfwSetWindowTitle(mWindow, title);
		mTitle = title;
	}

	int Window::GetWidth() const
	{
		return mWidth;
	}

	void Window::SetWidth(int width)
	{
		glfwSetWindowSize(mWindow, width, mHeight);
		mWidth = width;
	}

	int Window::GetHeight() const
	{
		return mHeight;
	}

	void Window::SetHeight(int height)
	{
		glfwSetWindowSize(mWindow, mWidth, height);
		mHeight = height;
	}

	bool Window::IsKeyDown(Keycode key)
	{
		return glfwGetKey(mWindow, (int)key);
	}

	bool Window::IsButtonDown(Buttoncode button)
	{
		return glfwGetMouseButton(mWindow, (int)button);
	}

	int Window::Hovered()
	{
		return glfwGetWindowAttrib(mWindow, GLFW_HOVERED);
	}

	void Window::OnUpdate()
	{
		PROFILER_FUNCTION();

		glfwPollEvents();
	}

	bool Window::ShouldQuit()
	{
		return glfwWindowShouldClose(mWindow);
	}

	bool Window::ShouldResizeWindow() const
	{
		return mShouldResizeWindow;
	}

	void Window::HintResizeWindow(bool value)
	{
		mShouldResizeWindow = value;
	}

	void Window::ToogleCursorMode(bool hide)
	{
		if(hide)
		{
			glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		}

		else
		{
			glfwSetInputMode(mWindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}

	int Window::CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface, const VkAllocationCallbacks* allocator)
	{
		return (int)glfwCreateWindowSurface(instance, mWindow, allocator, surface);
	}

	void Window::GetFramebufferSize(int* width, int* height)
	{
		glfwGetFramebufferSize(mWindow, width, height);
	}

	float Window::GetAspectRatio()
	{
		int width = 0;
		int height = 0;
		GetFramebufferSize(&width, &height);

		if (height == 0) // avoid division by 0
		{
			return 1.0f;
		}

		return (float)width / (float)height;
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

				if (action == GLFW_PRESS)
				{
					Application::Get()->OnKeyboardPress((Keycode)key);
				}

				else if (action == GLFW_RELEASE)
				{
					Application::Get()->OnKeyboardRelease((Keycode)key);
				}
			});

		// mouse button pressed
		glfwSetMouseButtonCallback(mWindow, [](GLFWwindow* window, int button, int action, int mods)
			{
				Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

				if (action == GLFW_PRESS)
				{
					Application::Get()->OnMousePress((Buttoncode)button);
				}
				
				else if (action == GLFW_RELEASE)
				{
					Application::Get()->OnMouseRelease((Buttoncode)button);
				}
			});

		// mouse scrolled
		glfwSetScrollCallback(mWindow, [](GLFWwindow* window, double x, double y)
			{
				Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

				Application::Get()->OnMouseScroll((float)y);
			});

		// mouse moved
		glfwSetCursorPosCallback(mWindow, [](GLFWwindow* window, double x, double y)
			{
				Window* win = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));

				float xPos = (float)x;
				float yPos = (float)y;

				if (win->GetData().mouseFirstMoved)
				{
					win->GetData().mouseLastX = xPos;
					win->GetData().mouseLastY = yPos;
					win->GetData().mouseFirstMoved = false;
				}

				float xOffset = xPos - win->GetData().mouseLastX;
				float yOffset = yPos - win->GetData().mouseLastY; // check reversed order also

				win->GetData().mouseLastX = xPos;
				win->GetData().mouseLastY = yPos;
				
				Application::Get()->OnMouseMove(xOffset, yOffset);
			});
	}
}