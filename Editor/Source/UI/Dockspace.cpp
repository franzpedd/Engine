#include "Dockspace.h"

namespace Cosmos
{
	Dockspace::Dockspace()
		: Entity("UI:Dockspace")
	{
		Logger() << "Creating Dockspace";
	}

	void Dockspace::OnUIDraw()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);
		
		const ImGuiWindowFlags wFlags = ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoNavFocus;
		
		ImGui::Begin("Dockspace", 0, wFlags);
		
		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));
		
		ImGui::End();
	}
}