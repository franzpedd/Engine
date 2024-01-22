#include "ModelBrowser.h"

namespace Cosmos
{
	ModelBrowser::ModelBrowser(std::shared_ptr<Renderer>& renderer)
		: mRenderer(renderer)
	{
		std::filesystem::path modelsPath = mRoot;
		modelsPath /= "Models";

		RefreshFilesExplorer(modelsPath.string());
	}

	ModelBrowser::~ModelBrowser()
	{
	}

	void ModelBrowser::OnUpdate()
	{
		if (!mShowWindow) return;
		
		ImGuiWindowFlags flags = {};
		flags |= ImGuiWindowFlags_HorizontalScrollbar;
		flags |= ImGuiWindowFlags_MenuBar;
		flags |= ImGuiWindowFlags_NoDocking;
		
		ImGui::Begin("Model Browser", &mShowWindow, flags);
		
		if (ImGui::BeginMenuBar())
		{
			ImGui::Text(ICON_FA_SEARCH " Search: ");
			ImGui::SameLine();
		
			char buffer[MAX_SEARCH_PATH_SIZE];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), mSearchStr.c_str(), sizeof(buffer));
		
			if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
			{
				mSearchStr = std::string(buffer);
			}
		
			if (ImGui::MenuItem("Refresh Folder"))
			{
				RefreshFilesExplorer(mRoot);
			}
		
			ImGui::EndMenuBar();
		}
		
		ImGui::Separator();
		ImGui::NewLine();
		
		ImVec2 currentPos = {};
		float offSet = 5.0f;
		
		for (auto& mdl : mModels)
		{
			if (mdl.find(mSearchStr) != std::string::npos)
			{
				if (ImGui::Selectable(mdl.c_str()))
				{
					mLastModelSelected = mdl;
					mShowWindow = false;
				}
			}
		}
		
		ImGui::End();
	}

	void ModelBrowser::ToogleOnOff(bool value)
	{
		if (value)
		{
			mShowWindow = true;
			return;
		}
		
		mShowWindow = false;
	}

	void ModelBrowser::RefreshFilesExplorer(std::string path)
	{
		mModels.clear();
		mRoot = path;
		uint32_t count = 0;
		
		for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(path))
		{
			std::string entryExtension = dirEntry.path().extension().string();
		
			for (size_t i = 0; i < EXTENSION_MAX; i++)
			{
				if (strcmp(mValidExtensions[i], entryExtension.c_str()) == 0)
				{
					count++;
					mModels.push_back(dirEntry.path().string());
				}
			}
		}
		
		LOG_TO_TERMINAL(Logger::Trace, "Found %d models in the Models Folder", count);
	}
}