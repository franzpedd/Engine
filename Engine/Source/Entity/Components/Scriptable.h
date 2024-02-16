#pragma once

#include "Entity/Scriptable/NativeScript.h"

namespace Cosmos
{
	// study using shared ptrs
	struct NativeScriptComponent
	{
	public:

		NativeScript* script = nullptr;

		// constructor
		NativeScriptComponent() = default;

	public:

		// function pointer to constructor
		NativeScript* (*CreateScriptFunc)();

		// function pointer to destructor
		void (*DestroyScriptFunc)();

		// assign pointers to functions
		template<typename T>
		void Bind()
		{
			// creates the script
			CreateScriptFunc = []() { return (NativeScript*)(new T()); };

			// deletes the script
			DestroyScriptFunc = [](NativeScriptComponent* component) { delete component->script; component->script = nullptr; };
		}
	};
}