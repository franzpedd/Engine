#pragma once

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

		// initialize random number generator
		inline void Seed(uint64_t new_seed = std::mt19937_64::default_seed) { mEngine.seed(new_seed); }

		// generate random number
		inline uint64_t Get() { return mEngine(); }

	private:

		std::mt19937_64 mEngine;
	};
}