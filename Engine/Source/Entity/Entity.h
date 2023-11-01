#pragma once

#include "Core/Application.h"
#include "Util/Keycodes.h"
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

		// returns it's id
		inline uint64_t ID() { return mID; }
		
	public:

		// returns how many instances of the derivated object exists
		virtual uint64_t GetInstancesCount() { return 0; };

		// for renderer drawing
		virtual void OnRenderDraw() {}

		// for logic updating
		virtual void OnUpdate(float timestep) {}

		// for freeing resources
		virtual void OnDestroy() {}

	public:

		// // called when the window is resized
		virtual void OnWindowResize() {}

		// called when a mouse position changes
		virtual void OnMouseMove(float x, float y) {}

		// called when a mouse scroll happens
		virtual void OnMouseScroll(float y) {}

		// called when a mouse button was pressed
		virtual void OnMousePress(Buttoncode button) {}

		// called when a mouse button was released
		virtual void OnMouseRelease(Buttoncode button) {}

		// called when a keyboard key is pressed
		virtual void OnKeyboardPress(Keycode key) {}

		// called when a keyboard key is released
		virtual void OnKeyboardRelease(Keycode key) {}

	protected:

		const char* mName;
		uint64_t mID = 0;
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