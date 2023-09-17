#include "Entity.h"

namespace Cosmos
{
	Entity::Entity(const char* name)
		: mName(name)
	{

	}

	Entity::~Entity()
	{

	}

	EntityStack::~EntityStack()
	{
		for (Entity* element : mEntities)
		{
			delete element;
		}
	}

	void EntityStack::PushOver(Entity* ent)
	{
		mEntities.emplace_back(ent);
	}

	void EntityStack::PopOver(Entity* ent)
	{
		auto it = std::find(mEntities.begin() + mMiddlePos, mEntities.end(), ent);
		if (it != mEntities.end())
		{
			mEntities.erase(it);
		}
	}

	void EntityStack::Push(Entity* ent)
	{
		mEntities.emplace(mEntities.begin() + mMiddlePos, ent);
		mMiddlePos++;
	}

	void EntityStack::Pop(Entity* ent)
	{
		auto it = std::find(mEntities.begin(), mEntities.begin() + mMiddlePos, ent);
		if (it != mEntities.begin() + mMiddlePos)
		{
			mEntities.erase(it);
			mMiddlePos--;
		}
	}
}