#pragma once

#include "Event.h"
#include "Platform/Input.h"

namespace Cosmos
{
    class KeyboardPressEvent : public Event
    {
    public:

        // constructor
        KeyboardPressEvent(Keycode key, bool shift = false)
            : Event("Keyboard Press", EventType::KeyboardPress), mKeycode(key), mShift(shift) {}

        // destructor
        ~KeyboardPressEvent() = default;

        // returns the key code 
        inline Keycode GetKeycode() const { return mKeycode; }

        // returns if shift operator was enabled
        inline bool WasShiftPressed() const { return mShift; }

    private:

        Keycode mKeycode; 
        bool mShift = false;
    };

    class KeyboardReleaseEvent : public Event
    {
    public:

        // constructor
        KeyboardReleaseEvent(Keycode key, bool shift = false)
            : Event("Keyboard Release", EventType::KeyboardRelease), mKeycode(key), mShift(shift) {}

        // destructor
        ~KeyboardReleaseEvent() = default;

        // returns the key code 
        inline Keycode GetKeycode() const { return mKeycode; }

        // returns if shift operator was enabled
        inline bool WasShiftPressed() const { return mShift; }
        
    private:

        Keycode mKeycode; 
        bool mShift = false;
    };

    class MousePressEvent : public Event
    {
    public:

        // constructor
        MousePressEvent(Buttoncode button)
            : Event("Mouse Press", EventType::MousePress), mButtonCode(button) {}

        // destructor
        ~MousePressEvent() = default;

        // returns the button code 
        inline Buttoncode GetButtoncode() const { return mButtonCode; }
        
    private:

        Buttoncode mButtonCode; 
    };

    class MouseReleaseEvent : public Event
    {
    public:

        // constructor
        MouseReleaseEvent(Buttoncode button)
            : Event("Mouse Release", EventType::MouseRelease), mButtonCode(button) {}

        // destructor
        ~MouseReleaseEvent() = default;

        // returns the button code 
        inline Buttoncode GetButtoncode() const { return mButtonCode; }
        
    private:

        Buttoncode mButtonCode; 
    };

    class MouseWheelEvent : public Event
    {
    public:

        // constructor
        MouseWheelEvent(int32_t delta)
            : Event("Mouse Wheel", EventType::MouseWheel), mDelta(delta) {}

        // destructor
        ~MouseWheelEvent() = default;

        // returns the delta move
        inline int32_t GetDelta() const { return mDelta; }

    private:

        int32_t mDelta;
    };

    class MouseMoveEvent : public Event
    {
    public:

        // constructor
        MouseMoveEvent(int32_t xOffset, int32_t yOffset)
            : Event("Mouse Move", EventType::MouseMove), mXOffset(xOffset), mYOffset(yOffset) {}

        // destructor
        ~MouseMoveEvent() = default;

        // returns the new x coodirnate
        inline int32_t GetXOffset() const { return mXOffset; }

        // returns the new y coordinates
        inline int32_t GetYOffset() const { return mYOffset; }

    private:

        int32_t mXOffset;
        int32_t mYOffset;
    };
}