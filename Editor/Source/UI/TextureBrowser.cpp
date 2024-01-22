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
		if (!mShowWindow) return;

		ImGuiWindowFlags flags = {};
		flags |= ImGuiWindowFlags_HorizontalScrollbar;
		flags |= ImGuiWindowFlags_MenuBar;
		flags |= ImGuiWindowFlags_NoDocking;

		ImGui::Begin("Texture Browser", &mShowWindow, flags);

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

			if (ImGui::BeginMenu("Display Size"))
			{
				if (ImGui::MenuItem("64x64"))
					mTextureSize = ImVec2(64.0f, 64.0f);
				
				if (ImGui::MenuItem("128x128"))
					mTextureSize = ImVec2(128.0f, 128.0f);
				
				if (ImGui::MenuItem("256x256"))
					mTextureSize = ImVec2(256.0f, 256.0f);
				
				ImGui::EndMenu();
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

		for (auto& tex : mTextures)
		{
			if (tex.path.string().find(mSearchStr) != std::string::npos)
			{
				if ((currentPos.x + (mTextureSize.x * 2)) <= ImGui::GetContentRegionAvail().x)
				{
					ImGui::SameLine();
				}

				ImGui::BeginGroup();
				
				currentPos = ImGui::GetCursorPos();
				ImGui::SetCursorPosX({ currentPos.x + offSet });
				
				if (ImGui::ImageButton(tex.descriptor, mTextureSize))
				{
					mLastSelectedTexture = tex;
					mShowWindow = false;
				}
				
				ImGui::Text(tex.path.filename().string().c_str());
				
				ImGui::EndGroup();
			}
		}

		ImGui::End();
	}

	void TextureBrowser::ToogleOnOff(bool value)
	{
		if (value)
		{
			mShowWindow = true;
			return;
		}

		mShowWindow = false;
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