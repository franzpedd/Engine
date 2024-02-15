#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>

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

		// constructor with str value
		UUID(std::string id);

		// destructor
		~UUID() = default;

		// returns the uuid
		inline uint64_t GetValue() const { return mUUID; }

		// returns the id
		operator uint64_t() const { return mUUID; }

		// used for using unordered map with hashing
		bool operator==(const UUID& id) const
		{
			if (this->mUUID == id) return true;
			else return false;
		}

		// used to stringfy uuid and use it as acessor in Scene entitymap
		operator std::string() const
		{
			return std::to_string(mUUID);
		}

	private:

		uint64_t mUUID;
	};
}