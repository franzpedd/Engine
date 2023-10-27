#include "Gizmo.h"

namespace Cosmos
{
	Gizmo::Gizmo(std::shared_ptr<Renderer>& renderer, Camera& camera)
		: Entity("Gizmo"), mRenderer(renderer), mCamera(camera)
	{
		Logger() << "Creating Gizmo";
	}

	void Gizmo::OnDraw()
	{
	}

	void Gizmo::OnUpdate(float timestep)
	{
	}

	void Gizmo::OnDrestroy()
	{
	}
}