#pragma once

namespace Cosmos
{
	class Timestep
	{
	public:

		// constructor
		Timestep(float time = 0.0f) : mTime(time) {}

		// destructor
		~Timestep() = default;

		// returns timestep in seconds (as casting as float)
		operator float() { return mTime; }

		// returns timestep in seconds
		inline float Seconds() const { return mTime; }

		// returns timestep in milli seconds
		inline float Milliseconds() const { return mTime * 1000.0f; }

	private:
		
		float mTime;
	};
}