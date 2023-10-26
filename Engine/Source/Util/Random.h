#pragma once

#include "Defines.h"
#include <random>

namespace Cosmos
{
	class Random
	{
	public:

		// constructor
		Random() = default;

		// destructor
		~Random() = default;

		// returns a pseudo random number
		static inline uint64_t GetRandomNumber() { return mDistribution(mEngine); }

	private:

		static std::random_device mDevice; // seed
		static std::mt19937_64 mEngine; // mersenne twister algorithm
		static std::uniform_int_distribution<uint64_t> mDistribution; // using unsigned long long distribution
	};

	class ID
	{
	public:

		// constructor
		ID()
		{
			mValue = Random::GetRandomNumber();
		}

		// destructor
		~ID() = default;
		
		// returns the id
		inline uint64_t Get() { return mValue; }

		// returns the id
		inline uint64_t operator()() { return mValue; }

	private:

		uint64_t mValue = 0;
	};
}