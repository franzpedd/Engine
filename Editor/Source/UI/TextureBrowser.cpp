#include "TextureBrowser.h"

namespace Cosmos
{
	TextureBrowser::TextureBrowser(std::shared_ptr<Renderer>& renderer)
		: mRenderer(renderer)
	{
		std::filesystem::path texturePath = mRoot;
		texturePath /= "Textures";

		RefreshFilesExplorer(texturePath.string());
	}

	TextureBrowser::~TextureBrowser()
	{
	}

	void TextureBrowser::OnUpdate()
	{
		ImGuiWindowFlags flags = {};
		//ImGuiWindowFlags_;

		flags |= ImGuiWindowFlags_HorizontalScrollbar;
		flags |= ImGuiWindowFlags_MenuBar;

		ImGui::Begin("Texture Browser", 0, flags);

		if (ImGui::BeginMenuBar())
		{
			ImGui::Text("Texture Name:");
			ImGui::Separator();
			ImGui::SameLine();

			char buffer[MAX_SEARCH_PATH_SIZE];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), mSearchStr.c_str(), sizeof(buffer));
		
			if (ImGui::InputText("##Name", buffer, sizeof(buffer)))
			{
				mSearchStr = std::string(buffer);
			}

			if (ImGui::SmallButton("Refresh"))
			{

			}
		
			ImGui::EndMenuBar();
		}

		auto& textures = SearchTextures(mSearchStr);

		ImVec2 currentPos = {};
		float offSet = 5.0f;

		for (auto& tex : textures)
		{
			// checks if must draw in the same line
			if ((currentPos.x + (mTextureSize.x * 2)) <= ImGui::GetContentRegionAvail().x)
				ImGui::SameLine();

			ImGui::BeginGroup();

			currentPos = ImGui::GetCursorPos();
			ImGui::SetCursorPos({ currentPos.x + offSet, currentPos.y });

			if (ImGui::ImageButton(tex.descriptor, mTextureSize))
			{
				
			}

			ImGui::Text(tex.path.filename().string().c_str());

			ImGui::EndGroup();
		}

		ImGui::End();
	}

	std::vector<TextureBrowser::TextureProperties> TextureBrowser::SearchTextures(std::string str)
	{
		std::vector<TextureBrowser::TextureProperties> textures;

		for (auto& tex : mTextures)
		{
			if (tex.path.string().find(str) != std::string::npos)
			{
				textures.emplace_back(tex);
			}
		}

		return textures;
	}

	void TextureBrowser::RefreshFilesExplorer(std::string path)
	{
		for (auto& tex : mTextures)
			RemoveTexture(tex.descriptor);

		mTextures.clear();
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

					TextureProperties tex;
					tex.path = dirEntry;
					tex.type = (Extension)i;
					tex.texture = Texture2D::Create(mRenderer->GetDevice(), dirEntry.path().string().c_str());
					tex.descriptor = AddTexture(tex.texture->GetSampler(), tex.texture->GetView(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
				
					mTextures.push_back(tex);
				}
			}
		}

		LOG_TO_TERMINAL(Logger::Trace, "Found %d textures in the Texture Folder", count);
	}
}