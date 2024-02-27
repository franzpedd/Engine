#pragma once

#include "Entity/Scriptable/NativeScript.h"

namespace Cosmos
{
	// study using shared ptrs
	struct NativeScriptComponent
	{
	public:

		entity::NativeScript* script;

		// constructor
		NativeScriptComponent() = default;
	};
}