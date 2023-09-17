#pragma once

#include <vector>

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

	public:

		// draws the entity (leave empty if doesnt required)
		virtual void OnDraw() = 0;

		// updates the entity (leave empty if doesnt required)
		virtual void OnUpdate() = 0;

		// called before destructor, for freeing resources
		virtual void OnDrestroy() = 0;

	private:

		const char* mName;
	};

	class EntityStack
	{
	public:

		// constructor
		EntityStack() = default;

		// destructor
		~EntityStack();

		// returns a reference to the elements vector
		inline std::vector<Entity*>& Entities() { return mEntities; }

	public:

		// pushes an ui to the top half of the stack
		void PushOver(Entity* ent);

		// pops an ui element from the top half of the stack
		void PopOver(Entity* ent);

		// pushes an ui to the bottom half of the stack
		void Push(Entity* ent);

		// pops an ui element from the bottom half of the stack
		void Pop(Entity* ent);

	public:

		// iterators
		std::vector<Entity*>::iterator begin() { return mEntities.begin(); }
		std::vector<Entity*>::iterator end() { return mEntities.end(); }
		std::vector<Entity*>::reverse_iterator rbegin() { return mEntities.rbegin(); }
		std::vector<Entity*>::reverse_iterator rend() { return mEntities.rend(); }
		std::vector<Entity*>::const_iterator begin() const { return mEntities.begin(); }
		std::vector<Entity*>::const_iterator end()	const { return mEntities.end(); }
		std::vector<Entity*>::const_reverse_iterator rbegin() const { return mEntities.rbegin(); }
		std::vector<Entity*>::const_reverse_iterator rend() const { return mEntities.rend(); }

	private:

		std::vector<Entity*> mEntities;
		uint32_t mMiddlePos = 0;
	};
}