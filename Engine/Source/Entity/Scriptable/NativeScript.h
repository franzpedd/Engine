#pragma once

namespace Cosmos
{
	// forward declarations
	class Entity;

	class NativeScript
	{
	public:

		// constructor
		NativeScript(Entity* entity, const char* name = "Script");

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