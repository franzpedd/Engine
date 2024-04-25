#pragma once

#include "Event.h"

namespace Cosmos
{
    class WindowCloseEvent : public Event
    {
    public:

        // constructor
        WindowCloseEvent()
            : Event("Window Close", EventType::WindowClose) {}

        // destructor
        ~WindowCloseEvent() = default;

    private:

    };

    class WindowResizeEvent : public Event
    {
    public:

        // constructor
        WindowResizeEvent(int32_t width, int32_t height)
            : Event("Window Resize", EventType::WindowResize), mWidth(width), mHeight(height) {}

        // destructor
        ~WindowResizeEvent() = default;

        // returns the new width 
        inline int32_t GetWidth() const { return mWidth; }

        // returns the new height
        inline int32_t GetHeight() const { return mHeight; }

    private:

        int32_t mWidth;
        int32_t mHeight;
    };
}