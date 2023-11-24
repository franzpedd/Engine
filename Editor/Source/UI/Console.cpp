#include "Console.h"

namespace Cosmos
{
	Console::Console()
	{
		Logger() << "Creating Console";

		Logger::Get().UseExternalConsole(true);
	}

	Console::~Console()
	{
	}

	void Console::OnUpdate()
	{
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.10f, 0.10f, 0.10f, 1.00f));

		ImGui::Begin("Console");

		ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", Logger::Get().GetMessages().str().c_str());

		ImGui::End();

		ImGui::PopStyleColor();
	}

	void Console::OnDestroy()
	{
	}
}