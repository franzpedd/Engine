#include "Console.h"

namespace Cosmos
{
	Console::Console()
	{
		Logger() << "Creating Console";
	}

	void Console::OnUpdate()
	{
		ImGui::Begin("Console", 0, ImGuiWindowFlags_HorizontalScrollbar);

		for (size_t i = 0; i < Logger::Get().GetMessages().size(); i++)
		{
			Logger::ConsoleMessage& msg = Logger::Get().GetMessages()[i];
			ImVec4 color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

			switch (msg.severity)
			{
			case Logger::Severity::Trace:
			{
				color = ImVec4(0.0f, 0.86f, 1.0f, 1.0f);
				ImGui::TextColored(color, ICON_FA_INFO_CIRCLE " %s", msg.message.c_str());

				break;
			}
			case Logger::Severity::Info:
			{
				color = ImVec4(0.0f, 0.5f, 0.6f, 1.0f);
				ImGui::TextColored(color, ICON_FA_INFO_CIRCLE " %s", msg.message.c_str());

				break;
			}
			case Logger::Severity::Todo:
			{
				color = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
				ImGui::TextColored(color, ICON_FA_INFO_CIRCLE " %s", msg.message.c_str());

				break;
			}
			case Logger::Severity::Warn:
			{
				color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
				ImGui::TextColored(color, ICON_FA_QUESTION_CIRCLE " %s", msg.message.c_str());

				break;
			}
			case Logger::Severity::Error:
			{
				color = ImVec4(1.0f, 0.65f, 0.0f, 1.0f);
				ImGui::TextColored(color, ICON_FA_QUESTION_CIRCLE " %s", msg.message.c_str());

				break;
			}
			}
		}

		ImGui::End();
	}
}