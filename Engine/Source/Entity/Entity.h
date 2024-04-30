#ifndef ENTITY_HEADER
#define ENTITY_HEADER

#include "Components/Base.h"
#include "Components/Renderable.h"
#include "Components/Scriptable.h"
#include "Components/Sound.h"

#include "Core/Application.h"
#include "Core/Scene.h"
#include "Debug/Logger.h"

#include "wrapper_entt.h"
#include <vector>

namespace Cosmos
{
	class Entity
	{
	public:

		// default constructor for unordered maps
		Entity() = default;

		// constructor with an id already assigned
		Entity(Scene* scene, entt::entity enttHandle, UUID id);

		// destructor
		~Entity() = default;

		// returns EnTT handle
		inline entt::entity GetHandle() const { return mEntityHandle; }

		// returns the entity uuid
		inline UUID GetUUID() { return mUUID; }


	public:

		// returns if entity have a valid id
		operator bool() const { return mEntityHandle != entt::null; }

		// cast entity id to uint32_t
		operator uint32_t() const { return (uint32_t)mEntityHandle; }

	public:

		// checks if entity has a certain component
		template<typename T>
		bool HasComponent()
		{
			return mScene->GetRegistryRef().all_of<T>(mEntityHandle);
		}

		// adds a component for the entity
		template<typename T, typename...Args>
		T& AddComponent(Args&&... args)
		{
			return mScene->GetRegistryRef().emplace_or_replace<T>(mEntityHandle, std::forward<Args>(args)...);
		}

		// returns the component
		template<typename T>
		T& GetComponent()
		{
			return mScene->GetRegistryRef().get<T>(mEntityHandle);
		}

		// removes the component
		template<typename T>
		void RemoveComponent()
		{
			mScene->GetRegistryRef().remove<T>(mEntityHandle);
		}

	protected:

		Scene* mScene;
		entt::entity mEntityHandle = entt::null;
		UUID mUUID = 0;
	};
}

#endif // ENTITY_HEADER