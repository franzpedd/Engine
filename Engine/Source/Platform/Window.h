#pragma once

#include "Defines.h"
#include "Input.h"

#include "Wrapper_sdl.h"

#include <chrono>
#include <vector>

namespace Cosmos
{
    class Window
    {
    public:

        // constructor
        Window(const char* title, uint32_t width, uint32_t height);

        // destructor
        ~Window();

        // returns the native window (sdl2)
        inline SDL_Window* GetNativeWindow() { return mWindow; }

    public:

        // updates the window 
        void OnUpdate();

        // returns the framebuffer size
        void GetFrameBufferSize(int32_t* width, int32_t* height);

        // returns the window's aspect ratio
        float GetAspectRatio();

        // returns if a keyboard keyis pressed
        bool IsKeyPressed(Keycode key);

        // returns if a mouse button is pressed
        bool IsButtonPressed(Buttoncode button);

        // enables or disables the cursor
        void ToggleCursor(bool hide);

    public:

        // returns if the application should exit
        inline bool ShouldQuit() const { return mShouldQuit; }

        // returns if the window should be resized
        inline bool ShouldResizeWindow() const { return mShouldResizeWindow; }

        // sets the window resize flag
        inline void HintResizeWindow(bool value) { mShouldResizeWindow = value; }

    public:

        // returns the window's title
        inline const char* GetTitle() { return mTitle; }

        // returns the window's width
        inline uint32_t GetWidth() const { return mWidth; }

        // returns the window's height
        inline uint32_t GetHeight() const { return mHeight; }

    private:

        const char* mTitle = nullptr;
        uint32_t mWidth = 0;
        uint32_t mHeight = 0;
        SDL_Window* mWindow = nullptr;
        bool mShouldQuit = false;
        bool mShouldResizeWindow = false;
    };

    class FramesPerSecond
    {
    public:

        // constructor
        FramesPerSecond() = default;

        // destructor
        ~FramesPerSecond() = default;

        // returns the average fps count
        inline uint32_t GetFPS() const { return mLastFPS; }

        // returns the timestep
        inline float GetTimestep() const { return mTimestep; }

    public:

        // starts the frames per second count
        void StartFrame();

        // ends the frames per second count
        void EndFrame();

    private:

        std::chrono::high_resolution_clock::time_point mStart;
        std::chrono::high_resolution_clock::time_point mEnd;
        double mTimeDiff = 0.0f;
        float mFpsTimer = 0.0f;
        uint32_t mFrames = 0; // average fps
        float mTimestep = 1.0f; // timestep/delta time (used to update logic)
        uint32_t mLastFPS = 0;
    };
}