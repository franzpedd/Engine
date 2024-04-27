#include "epch.h"
#include "Window.h"

#include "Core/Application.h"
#include "Event/InputEvent.h"
#include "Event/WindowEvent.h"
#include "Util/Memory.h"

#include "UI/GUI.h"

namespace Cosmos
{
    Window::Window(const char* title, uint32_t width, uint32_t height)
    {
        mTitle = title;
        mWidth = width;
        mHeight = height;

        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER) != 0)
        {
            LOG_TO_TERMINAL(Logger::Error, "Failed to initialize SDL2");
            return;
        }

        SDL_SetHint(SDL_HINT_IME_SHOW_UI, "1");

        mWindow = SDL_CreateWindow
        (
            mTitle,
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            mWidth,
            mHeight,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
        );

        LOG_ASSERT(mWindow != nullptr, "Window could not be created. Error: %s", SDL_GetError());
    }

    Window::~Window()
    {
        SDL_DestroyWindow(mWindow);
        SDL_Quit();
    }

    void Window::OnUpdate()
    {
        SDL_Event e;

        while (SDL_PollEvent(&e))
        {
            GUI::HandleInternalEvent(&e);

            switch (e.type)
            {
                // input
                case SDL_KEYDOWN:
                {
                    Shared<KeyboardPressEvent> event = CreateShared<KeyboardPressEvent>((Keycode)e.key.keysym.sym, e.key.keysym.mod = KMOD_NONE ? true : false);
                    Application::GetInstance()->OnEvent(event);
                    break;
                }

                case SDL_KEYUP:
                {
                    Shared<KeyboardReleaseEvent> event = CreateShared<KeyboardReleaseEvent>((Keycode)e.key.keysym.sym, e.key.keysym.mod = KMOD_NONE ? true : false);
                    Application::GetInstance()->OnEvent(event);
                    break;
                }

                case SDL_MOUSEBUTTONDOWN:
                {
                    Shared<MousePressEvent> event = CreateShared<MousePressEvent>((Buttoncode)e.button.button);
                    Application::GetInstance()->OnEvent(event);
                    break;
                }

                case SDL_MOUSEBUTTONUP:
                {
                    Shared<MouseReleaseEvent> event = CreateShared<MouseReleaseEvent>((Buttoncode)e.button.button);
                    Application::GetInstance()->OnEvent(event);
                    break;
                }

                case SDL_MOUSEWHEEL:
                {
                    Shared<MouseWheelEvent> event = CreateShared<MouseWheelEvent>(e.wheel.y);
                    Application::GetInstance()->OnEvent(event);
                    break;
                }

                case SDL_MOUSEMOTION:
                {
                    Shared<MouseMoveEvent> event = CreateShared<MouseMoveEvent>(e.motion.xrel, e.motion.yrel);
                    Application::GetInstance()->OnEvent(event);
                    break;
                }

                // window
                case SDL_QUIT:
                {
                    mShouldQuit = true;

                    Shared<WindowCloseEvent> event = CreateShared<WindowCloseEvent>();
                    Application::GetInstance()->OnEvent(event);
                    break;
                }

                case SDL_WINDOWEVENT:
                {
                    if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
                    {
                        mShouldResizeWindow = true;

                        Shared<WindowResizeEvent> event = CreateShared<WindowResizeEvent>(e.window.data1, e.window.data2);
                        Application::GetInstance()->OnEvent(event);
                        break;
                    }
                }
            }
        }
    }

    void Window::GetFrameBufferSize(int32_t* width, int32_t* height)
    {
        SDL_GL_GetDrawableSize(mWindow, width, height);
    }

    float Window::GetAspectRatio()
    {
        int32_t width = 0;
        int32_t height = 0;
        GetFrameBufferSize(&width, &height);

        if (height == 0) // avoid division by 0
        {
            return 1.0f;
        }

        float aspect = ((float)width / (float)height);
        return aspect;
    }

    bool Window::IsKeyPressed(Keycode key)
    {
        const uint8_t* keys = SDL_GetKeyboardState(nullptr);

        return keys[(SDL_Scancode)key];
    }

    bool Window::IsButtonPressed(Buttoncode button)
    {
        int32_t x, y;

        if (SDL_GetMouseState(&x, &y) & SDL_BUTTON((uint32_t)button))
        {
            return true;
        }

        return false;
    }

    void Window::ToggleCursor(bool hide)
    {
        if (hide)
        {
            SDL_ShowCursor(SDL_DISABLE);
            SDL_SetRelativeMouseMode(SDL_TRUE);
        }

        else
        {
            SDL_ShowCursor(SDL_ENABLE);
            SDL_SetRelativeMouseMode(SDL_FALSE);
        }
    }

    void FramesPerSecond::StartFrame()
    {
        mStart = std::chrono::high_resolution_clock::now();
    }

    void FramesPerSecond::EndFrame()
    {
        mEnd = std::chrono::high_resolution_clock::now();	// ends timer
        mFrames++;											// add frame to the count

        // calculates time taken by the renderer updating
        mTimeDiff = std::chrono::duration<double, std::milli>(mEnd - mStart).count();

        mTimestep = (float)mTimeDiff / 1000.0f; // timestep

        // calculates time taken by last timestamp and renderer finished
        mFpsTimer += (float)mTimeDiff;

        if (mFpsTimer > 1000.0f) // greater than next frame, reset frame counting
        {
            mLastFPS = (uint32_t)((float)mFrames * (1000.0f / mFpsTimer));
            mFrames = 0;
            mFpsTimer = 0.0f;
        }
    }

}