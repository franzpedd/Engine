#pragma once

#include "Components.h"

#include "Core/Application.h"
#include "Core/Scene.h"
#include "Util/EnTT.h"
#include "Util/Keycodes.h"

#include <vector>

namespace Cosmos
{
	// forward declaration
	class Scene;

	class Entity
	{
	public:

		// constructor, used for temporary entities
		Entity() = default;

		// constructor, used for specialized entities
		Entity(Scene* scene);

		// constructor with an id already assigned
		Entity(Scene* scene, entt::entity id);

		// copy constructor
		Entity(const Entity& other) = default;

		// destructor
		virtual ~Entity() = default;

		// selects/unselects entity, for editor moving
		inline void SetSelected(bool value) { mSelected = value; }

		// returns the entity uuid
		inline UUID GetUUID() { return GetComponent<IDComponent>().id; }

	public:

		// returns the Cosmos::Entity has the entt::entity
		operator entt::entity() const { return mEntityHandle; }

		// checks if a given other entity object is the same (has same id)
		bool operator==(const Entity& other) const
		{
			return mEntityHandle == other.mEntityHandle && mScene == other.mScene;
		}

		// checks if a given other entity object is different (pointers are different)
		bool operator!=(const Entity& other) const
		{
			return !(*this == other);
		}

		// cast entity id to uint32_t
		operator uint32_t() const { return (uint32_t)mEntityHandle; }

		// returns if entity have a valid id
		operator bool() const { return mEntityHandle != entt::null; }

	public:

		// checks if entity has a certain component
		template<typename T>
		bool HasComponent()
		{
			return mScene->Registry().all_of<T>(mEntityHandle);
		}

		// adds a component for the entity
		template<typename T, typename...Args>
		T& AddComponent(Args&&... args)
		{
			return mScene->Registry().emplace_or_replace<T>(mEntityHandle, std::forward<Args>(args)...);
		}

		// returns the component
		template<typename T>
		T& GetComponent()
		{
			return mScene->Registry().get<T>(mEntityHandle);
		}

		// removes the component
		template<typename T>
		void RemoveComponent()
		{
			mScene->Registry().remove<T>(mEntityHandle);
		}

	public:

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

		bool mSelected = false;
		entt::entity mEntityHandle = entt::null;
		Scene* mScene = nullptr;
	};

	class EntityStack
	{
	public:

		// returns a smart-ptr to an entity stack
		static std::unique_ptr<EntityStack> Create();

		// constructor
		EntityStack() = default;

		// destructor
		~EntityStack();

		// returns a reference to the elements vector
		inline std::vector<Entity*>& GetEntitiesVector() { return mEntities; }

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