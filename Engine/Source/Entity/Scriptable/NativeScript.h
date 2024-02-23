#pragma once

#include "Core/Scene.h"
#include "Entity/Entity.h"

#include <memory>
#include <string>

namespace Cosmos
{
	class NativeScript
	{
	public:

		// constructor
		NativeScript() = default;

		// destructor
		virtual ~NativeScript() = default;

	protected:

		// called right after the script is created
		virtual void OnCreate() {};

		// called on the frame update routine
		virtual void OnUpdate(float timestep) {};

		// called when the script is about to be deleted
		virtual void OnDestroy() {};

	public:

		// returns entity the component
		template<typename T>
		T& HasComponent() { return mEntity.HasComponent<T>(); }

		// returns entity the component
		template<typename T>
		T& GetComponent() { return mEntity.GetComponent<T>(); }

	protected:

		friend class Scene;
		Entity* mEntity = nullptr;
	};
}