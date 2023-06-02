#pragma once

#include "Core/Defines.h"

#include <memory>

namespace Cosmos
{
	class COSMOS_API Clock
	{
	public:

		// returns a smart-ptr to a new clock
		static std::shared_ptr<Clock> Create();

		// constructor
		Clock() = default;

		// destructor
		~Clock() = default;

		// starts the clock
		void Start();

		// stops the clock
		void Stop();

		// updates the clock, should be called before checking elapsed time
		void Update();

		// returns the clock's start time
		inline f64 GetStartTime() { return m_StartTime; }

		// sets the clock's start time
		inline void SetStartTime(f64 time) { m_StartTime = time; }

		// returns the clock's elapsed time
		inline f64 GetElapsedTime() { return m_ElapsedTime; }

		// sets the clock's elapsed time
		inline void SetElapsedTime(f64 time) { m_ElapsedTime = time; }

	private:

		f64 m_StartTime{};
		f64 m_ElapsedTime{};
	};
}