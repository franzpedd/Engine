#pragma once

#include "Defines.h"

#include <string>

namespace Cosmos
{
    enum EventType
    {
        Undefined = -1,

        // input events
        KeyboardPress,
        KeyboardRelease,
        MousePress,
        MouseRelease,
        MouseWheel,
        MouseMove,

        // window events
        WindowClose,
        WindowResize,

        EventMax
    };

    class Event
    {
    public:

        // constructor
        Event(std::string name = "Event", EventType type = EventType::Undefined)
            : mName(name), mType(type) {}

        // destructor
        virtual ~Event() = default;

        // returns the name
        inline std::string GetName() { return mName; }

        // returns the event type
        inline EventType GetType() const { return mType; }

    private:

        std::string mName;
        EventType mType;
    };
}