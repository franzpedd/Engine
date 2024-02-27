#pragma once

#include "Core/Scene.h"

namespace Cosmos::entity
{
	class NativeScript
	{
	public:

		// constructor
		NativeScript(const char* name = "Script", Entity * entity = nullptr);

		// destructor
		virtual ~NativeScript();

	public:

		// called on every update tick
		virtual void OnUpdate(float timestep) {};

	protected:

		const char* mName = {};
		Entity* mEntity = nullptr;
	};
}