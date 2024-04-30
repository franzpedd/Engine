#pragma once

#include "Entity/Scriptable/NativeScript.h"

namespace Cosmos
{
	// study using shared ptrs
	struct NativeScriptComponent
	{
	public:

		NativeScript* script;

		// constructor
		NativeScriptComponent() = default;
	};
}