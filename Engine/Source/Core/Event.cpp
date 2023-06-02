#include "Event.h"

#include "Util/Logger.h"

namespace Cosmos
{
	EventSystem* EventSystem::s_EventSystem = nullptr;

	std::shared_ptr<EventSystem> EventSystem::Create()
	{
		return std::make_shared<EventSystem>();
	}

	EventSystem::EventSystem()
	{
		LOG_ASSERT(s_EventSystem == nullptr, "There is an Event System already");
		s_EventSystem = this;

		for (u64 i = 0; i < MAX_REGISTERED_EVENTS; i++)
		{
			m_RegisteredEvents[i].Events.clear();
		}
	}

	EventSystem::~EventSystem()
	{
		for (u64 i = 0; i < MAX_REGISTERED_EVENTS; i++)
		{
			m_RegisteredEvents[i].Events.clear();
		}

		s_EventSystem = nullptr;
	}

	bool EventSystem::Register(EventType eventcode, void* listener, PFN_OnEvent func)
	{
		LOG_ASSERT(s_EventSystem != nullptr, "The event system is offline");

		// checks if event was registered already
		for (u64 i = 0; i < m_RegisteredEvents[eventcode].Events.size(); i++)
		{
			if (m_RegisteredEvents[eventcode].Events[i]->GetListener() == listener)
			{
				LOG_WARNING("The listener for this event was previously registered");
				return false;
			}
		}

		// register event
		m_RegisteredEvents[eventcode].Events.push_back(new Event{ listener, func });

		return true;
	}

	bool EventSystem::Unregister(EventType eventcode, void* listener, PFN_OnEvent func)
	{
		LOG_ASSERT(s_EventSystem != nullptr, "The event system is offline");

		if (m_RegisteredEvents[eventcode].Events.empty())
		{
			LOG_WARNING("Nothing was ever registered with the given code");
			return false;
		}

		// find event
		for (u64 i = 0; i < m_RegisteredEvents[eventcode].Events.size(); i++)
		{
			Event* e = m_RegisteredEvents[eventcode].Events[i];

			if (e->GetListener() == listener && e->GetCallback() == func)
			{
				m_RegisteredEvents[eventcode].Events.erase(m_RegisteredEvents[eventcode].Events.begin() + i);
				return true;
			}
		}

		LOG_WARNING("Event unregister was not a match to a valid event");
		return false;
	}

	bool EventSystem::Fire(EventType eventcode, void* sender, EventData data)
	{
		LOG_ASSERT(s_EventSystem != nullptr, "The event system is offline");

		if (m_RegisteredEvents[eventcode].Events.empty())
		{
			return false;
		}

		// search the event and fire it
		for (u64 i = 0; i < m_RegisteredEvents[eventcode].Events.size(); i++)
		{
			Event* e = m_RegisteredEvents[eventcode].Events[i];
			PFN_OnEvent callback = e->GetCallback();

			if (callback(eventcode, sender, e->GetListener(), data))
			{
				return true;
			}
		}

		return false;
	}
}