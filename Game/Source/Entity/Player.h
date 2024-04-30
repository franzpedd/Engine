#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Player : public NativeScript
	{
	public:

		// constructor
		Player(Entity * entity = nullptr);

		// destructor
		virtual ~Player();

	public:

		// called on every game loop iteration
		virtual void OnUpdate(float timestep) override;
	
	};
}