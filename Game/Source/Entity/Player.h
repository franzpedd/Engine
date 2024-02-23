#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Player
	{
	public:

		// constructor
		Player() = default;

		~Player() = default;

	public:

		// called after the Application constructor is called
		void OnCreate();

		// called on every game loop iteration
		void OnUpdate(float timestep);

		// called once before the Application destructor is called
		void OnDestroy();

	private:


	};
}