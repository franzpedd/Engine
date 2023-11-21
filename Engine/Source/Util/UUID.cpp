#include "UUID.h"

#include <random>

namespace Cosmos
{
	static std::random_device sDevice;
	static std::mt19937_64 sEngine(sDevice());
	static std::uniform_int_distribution<uint64_t> sDistribution;

	UUID::UUID()
		: mUUID(sDistribution(sEngine))
	{
	}

	UUID::UUID(uint64_t id)
		: mUUID(id)
	{
	}
}