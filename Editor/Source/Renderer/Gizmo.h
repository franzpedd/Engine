#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Gizmo : public Entity
	{
	public:

		// constructor
		Gizmo(std::shared_ptr<Renderer>& renderer, Camera& camera);

		// destructor
		~Gizmo() = default;

	public:

		// draws the entity (leave empty if doesnt required)
		virtual void OnDraw() override;

		// updates the entity (leave empty if doesnt required)
		virtual void OnUpdate(float timestep) override;

		// called before destructor, for freeing resources
		virtual void OnDrestroy() override;

	private:

		std::shared_ptr<Renderer>& mRenderer;
		Camera& mCamera;
	};

}