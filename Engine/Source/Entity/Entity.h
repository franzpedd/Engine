#pragma once

#include "Components/Base.h"
#include "Components/Renderable.h"
#include "Components/Scriptable.h"

#include "Core/Application.h"
#include "Core/Scene.h"
#include "Debug/Logger.h"
#include "Entity/EnTT.h"
#include "Platform/Keycodes.h"

#include <vector>

namespace Cosmos
{
	class Entity
	{
	public:

		// default constructor for unordered maps
		Entity() = default;

		// constructor with an id already assigned
		Entity(Scene* scene, entt::entity id) : mScene(scene), mEntityHandle(id) {}

		// destructor
		virtual ~Entity() = default;

		// returns EnTT handle
		inline entt::entity GetHandle() const { return mEntityHandle; }

		// returns the entity uuid
		inline UUID GetUUID() { return GetComponent<IDComponent>().id; }

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
			if (mScene == nullptr)
			{
				LOG_TO_TERMINAL(Logger::Error, "Scene is nullptr");
			}

			return mScene->Registry().all_of<T>(mEntityHandle);
		}

		// adds a component for the entity
		template<typename T, typename...Args>
		T& AddComponent(Args&&... args)
		{
			if (mScene == nullptr)
			{
				LOG_TO_TERMINAL(Logger::Error, "Scene is nullptr");
			}	

			return mScene->Registry().emplace_or_replace<T>(mEntityHandle, std::forward<Args>(args)...);
		}

		// returns the component
		template<typename T>
		T& GetComponent()
		{
			if (mScene == nullptr)
			{
				LOG_TO_TERMINAL(Logger::Error, "Scene is nullptr");
			}

			return mScene->Registry().get<T>(mEntityHandle);
		}

		// removes the component
		template<typename T>
		void RemoveComponent()
		{
			if (mScene == nullptr)
			{
				LOG_TO_TERMINAL(Logger::Error, "Scene is nullptr");
			}

			mScene->Registry().remove<T>(mEntityHandle);
		}

	protected:

		Scene* mScene = nullptr;
		bool mSelected = false;
		entt::entity mEntityHandle = entt::null;
	};
}