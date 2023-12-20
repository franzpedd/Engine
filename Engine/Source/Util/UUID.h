#pragma once

#include <cstdint>
#include <unordered_map>

namespace Cosmos
{
	class UUID
	{
	public:

		struct Hash
		{
			size_t operator()(const UUID& id) const
			{
				size_t hash = std::hash<uint64_t>()(id.mUUID);
				return hash;
			}
		};

	public:

		// constructor
		UUID();

		// constructor with value
		UUID(uint64_t id);

		// destructor
		~UUID() = default;

		// returns the uuid
		inline uint64_t GetValue() { return mUUID; }

		// returns the id
		operator uint64_t() const { return mUUID; }

		// used for using unordered map with hashing
		bool operator==(const UUID& id) const
		{
			if (this->mUUID == id) return true;
			else return false;
		}

	private:

		uint64_t mUUID;
	};
}