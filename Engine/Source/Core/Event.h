#pragma once

#include "Core/Defines.h"

#include <memory>
#include <vector>

#define MAX_REGISTERED_EVENTS 512

namespace Cosmos
{
	typedef enum EventType
	{
		PLATFORM_QUIT = 0,
		PLATFORM_RESIZE,
		PLATFORM_FOCUS,
		PLATFORM_UNFOCUS,

		KEY_PRESSED,
		KEY_RELEASED,
		BUTTON_PRESSED,
		BUTTON_RELEASED,
		MOUSE_MOVED,
		MOUSE_WHEELED,

		MAX_EVENT_TYPES
	} EventType;

	// holds information about the event
	class COSMOS_API EventData
	{
	public:

		union // data is a messages of 128 bytes
		{
			// two 64 bytes info
			i64 i64[2];
			u64 u64[2];
			f64 f64[2];

			// four 32 bytes info
			i32 i32[4];
			u32 u32[4];
			f32 f32[4];

			// eight 16 bytes info
			i16 i16[8];
			u16 u16[8];

			// sixteen 8 bytes info
			i8 i8[16];
			u8 u8[16];
			char c[16];

		} data{};

	public:

		// constructor
		EventData() = default;

		// destructor
		virtual ~EventData() = default;
	};

	// pointer-to-function of an event
	typedef bool(*PFN_OnEvent)(EventType type, void* sender, void* listener, EventData data);

	class COSMOS_API Event
	{
	public:

		// constructor
		Event(void* listener, PFN_OnEvent callback)
			: m_Listener(listener), m_Callback(callback)
		{

		}

		// destructor
		virtual ~Event() = default;

		// returns the event's listener
		inline void* GetListener() { return m_Listener; }

		// returns the event's callback
		inline PFN_OnEvent GetCallback() { return m_Callback; }

	private:

		void* m_Listener;
		PFN_OnEvent m_Callback;
	};

	class COSMOS_API EventSystem
	{
	public:

		// allows multiple events
		struct RegisteredEvent
		{
			std::vector<Event*> Events;
		};

	public:

		// creates the event system
		static std::shared_ptr<EventSystem> Create();

		// constructor
		EventSystem();

		// destructor
		virtual ~EventSystem();

		// returns the application singleton
		static inline EventSystem& Get() { return *s_EventSystem; }

	public:

		// register an event
		bool Register(EventType eventcode, void* listener, PFN_OnEvent func);

		// register an event
		bool Unregister(EventType eventcode, void* listener, PFN_OnEvent func);

		// register an event
		bool Fire(EventType eventcode, void* sender, EventData data);

	private:

		static EventSystem* s_EventSystem;
		RegisteredEvent m_RegisteredEvents[MAX_REGISTERED_EVENTS];
	};
}

// macros to facilitate how register/unregister and fire events
#define EVENT_REGISTER(code, listener, func) { Cosmos::EventSystem::Get().Register(code, listener, func); }
#define EVENT_UNREGISTER(code, listener, func) { Cosmos::EventSystem::Get().Unregister(code, listener, func); }
#define EVENT_FIRE(code, listener, data) Cosmos::EventSystem::Get().Fire(code, listener, data);