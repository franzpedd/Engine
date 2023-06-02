#include "Clock.h"

#include "Platform/Platform.h"

namespace Cosmos
{
	std::shared_ptr<Clock> Clock::Create()
	{
		return std::make_shared<Clock>();
	}

	void Clock::Start()
	{
		m_StartTime = GetTime();
		m_ElapsedTime = 0;
	}

	void Clock::Stop()
	{
		m_StartTime = 0;
	}

	void Clock::Update()
	{
		if (m_StartTime != 0)
		{
			m_ElapsedTime = GetTime() - m_StartTime;
		}
	}
}