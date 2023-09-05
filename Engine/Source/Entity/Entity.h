#pragma once

namespace Cosmos
{
	class Entity
	{
	public:

		// constructor
		Entity(const char* name);

		// destructor
		virtual ~Entity();
		
		// returns it's name
		inline const char* Name() { return mName; }
	
	private:

		const char* mName;
	};
}