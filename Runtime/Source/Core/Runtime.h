#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Runtime : public Cosmos::Application
	{
	public:

		// constructor
		Runtime();

		// destructor
		virtual ~Runtime();

	public:

		// used to setup initial resources
		virtual void OnInitialize() override;

		// updates the logic
		virtual void OnUpdate(f32 timestep) override;

		// updates the render
		virtual void OnRender(f32 timestep) override;

		// free used resources
		virtual void OnTerminate() override;

	private:

	};
}