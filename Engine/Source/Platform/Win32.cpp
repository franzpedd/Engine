#include "Platform.h"

#if defined(PLATFORM_WINDOWS)

#include "Core/Input.h"
#include "Util/Logger.h"

#include <windows.h>
#include <windowsx.h>

namespace Cosmos
{
	struct PlatformWin32
	{
	public:

		// constructor
		PlatformWin32() = default;

		// destructor
		~PlatformWin32() = default;

	public:

		HINSTANCE HInstance;
		HWND HWind;
		f64 ClockFrequency;
		LARGE_INTEGER StarTime;
	};

	static PlatformWin32* s_Platform;

	// forward declare os-messages function
	LRESULT CALLBACK WindowsProcessMessages(HWND hWnd, u32 msg, WPARAM wParam, LPARAM lParam);

	std::shared_ptr<Platform> Platform::Create(const char* title, u32 width, u32 height, u32 xpos, u32 ypos)
	{
		return std::make_shared<Platform>(title, width, height, xpos, ypos);
	}

	Platform::Platform(const char* title, u32 width, u32 height, u32 xpos, u32 ypos)
		: m_Title(title), m_Width(width), m_Height(height), m_XPos(xpos), m_YPos(ypos)
	{
		// create win32 api platform
		s_Platform = new PlatformWin32;
		LOG_ASSERT(s_Platform != nullptr, "The platform state was a nullptr");
		s_Platform->HInstance = GetModuleHandleA(0);

		// create window class
		WNDCLASSA wc;
		std::memset(&wc, 0, sizeof(wc));

		wc.style = CS_DBLCLKS;
		wc.lpfnWndProc = WindowsProcessMessages;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = s_Platform->HInstance;
		wc.hIcon = LoadIcon(s_Platform->HInstance, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszClassName = "Cosmos_WC";
		
		LOG_ASSERT(RegisterClassA(&wc), "The window class registration has failed");

		// setup window style
		u32 windowExStyle = WS_EX_APPWINDOW;
		u32 windowStyle = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
		windowStyle |= WS_MAXIMIZEBOX;
		windowStyle |= WS_MINIMIZEBOX;
		windowStyle |= WS_THICKFRAME;

		// adjust window position (consider window decorations)
		RECT borderRect = { 0, 0, 0, 0 };
		AdjustWindowRectEx(&borderRect, windowStyle, 0, windowExStyle);

		m_XPos += borderRect.left;
		m_YPos += borderRect.top;
		m_Width += borderRect.right - borderRect.left;
		m_Height += borderRect.bottom - borderRect.top;

		// create window object
		HWND handle = CreateWindowExA
		(
			windowExStyle,
			"Cosmos_WC",
			m_Title,
			windowStyle,
			m_XPos,
			m_YPos,
			m_Width,
			m_Height,
			0,
			0,
			s_Platform->HInstance,
			0
		);

		LOG_ASSERT(handle != nullptr, "The window creation has failed");
		s_Platform->HWind = handle;

		// show window
		bool startActive = 1;
		i32 startFlags = startActive ? SW_SHOW : SW_SHOWNOACTIVATE;
		ShowWindow(s_Platform->HWind, startFlags);

		// setup clock
		LARGE_INTEGER frequency;
		QueryPerformanceFrequency(&frequency);
		s_Platform->ClockFrequency = 1.0 / (f64)frequency.QuadPart;
		QueryPerformanceCounter(&s_Platform->StarTime);
	}

	Platform::~Platform()
	{
		delete s_Platform;
	}

	bool Platform::ProcessMessages()
	{
		MSG message;

		while (PeekMessageA(&message, s_Platform->HWind, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessageA(&message);
		}

		return true;
	}

	void Platform::Sleep(u64 ms)
	{
		SleepEx(static_cast<DWORD>(ms), TRUE);
	}

	f64 GetTime()
	{
		LARGE_INTEGER now;
		QueryPerformanceCounter(&now);

		return (f64)now.QuadPart * s_Platform->ClockFrequency;
	}

	void OutputMessageToConsole(const char* message, u8 color)
	{
		HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
		u8 levels[6] = { 8, 1, 2, 6, 4, 64 };

		SetConsoleTextAttribute(consoleHandle, levels[color]);
		OutputDebugStringA(message);

		u64 length = strlen(message);
		LPDWORD number_written = 0;

		WriteConsoleA(GetStdHandle(STD_OUTPUT_HANDLE), message, (DWORD)length, number_written, 0);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	}

	void OutputErrorToConsole(const char* message, u8 color)
	{
		HANDLE consoleHandle = GetStdHandle(STD_ERROR_HANDLE);
		u8 levels[6] = { 8, 1, 2, 6, 4, 64 };

		SetConsoleTextAttribute(consoleHandle, levels[color]);
		OutputDebugStringA(message);

		u64 length = strlen(message);
		LPDWORD number_written = 0;

		WriteConsoleA(GetStdHandle(STD_ERROR_HANDLE), message, (DWORD)length, number_written, 0);
		SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15);
	}

	LRESULT CALLBACK WindowsProcessMessages(HWND hWnd, u32 msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
			case WM_ERASEBKGND: // notify the OS that erasing will be handled by the application to prevent flicker.
			{
				return 1;
			}

			case WM_CLOSE: // application quit
			{
				EventData e;
				EVENT_FIRE(EventType::PLATFORM_QUIT, 0, e);

				return 0;
			}

			case WM_DESTROY: // application terminated
			{
				PostQuitMessage(0);
				return 0;
			}

			case WM_SIZE: // application resized
			{
				break;
			}

			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
			case WM_KEYUP:
			case WM_SYSKEYUP:
			{
				bool pressed = (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN);

				Input::Keycode key = (Input::Keycode)wParam;
				Input::Get().HandleKey(key, pressed);

				break;
			}

			case WM_MOUSEMOVE:
			{
				i32 x = GET_X_LPARAM(lParam);
				i32 y = GET_Y_LPARAM(lParam);

				Input::Get().HandleMove(x, y);
				
				break;
			}

			case WM_MOUSEWHEEL:
			{
				i32 delta = GET_WHEEL_DELTA_WPARAM(wParam);
				
				if (delta != 0)
				{
					delta = (delta < 0) ? -1 : 1;

					Input::Get().HandleWheel(delta);
				}
				
				break;
			}

			case WM_LBUTTONDOWN:
			case WM_MBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_LBUTTONUP:
			case WM_MBUTTONUP:
			case WM_RBUTTONUP:
			{
				bool pressed = msg == WM_LBUTTONDOWN || msg == WM_RBUTTONDOWN || msg == WM_MBUTTONDOWN;
				
				switch (msg)
				{
					case WM_LBUTTONDOWN:
					case WM_LBUTTONUP:
					{
						Input::Get().HandleButton(Input::BUTTON_LEFT, pressed);
						break;
					}
				
					case WM_MBUTTONDOWN:
					case WM_MBUTTONUP:
					{
						Input::Get().HandleButton(Input::BUTTON_MIDDLE, pressed);
						break;
					}
				
					case WM_RBUTTONDOWN:
					case WM_RBUTTONUP:
					{
						Input::Get().HandleButton(Input::BUTTON_RIGHT, pressed);
						break;
					}
				}
				
				break;
			}

		}

		return DefWindowProcA(hWnd, msg, wParam, lParam);
	}
}

#endif