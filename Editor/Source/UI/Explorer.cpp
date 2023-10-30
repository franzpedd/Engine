#include "Explorer.h"

namespace Cosmos
{
	Explorer::Explorer(std::shared_ptr<Renderer>& renderer)
		: Entity("UI:Explorer"), mRenderer(renderer)
	{
		CreateTextures();
	}

	void Explorer::OnUIDraw()
	{
		std::vector<Explorer::ItemProperties> currDirItems = RefreshExplorer(mRoot, true);
		ImVec2 buttonSize = { 50, 50 };
		ImVec2 currentPos = {};
		float offSet = 5.0f;

		std::filesystem::path parentPath = mRoot;
		parentPath = parentPath.parent_path();

		ImGui::Begin("Explorer");
		ImGui::BeginChild("Items");

		// draw back folder
		{
			ImGui::BeginGroup();

			if (ImGui::ImageButton("...", mFolderDescriptorSet, buttonSize))
			{
				ItemProperties item;
				item.descriptor = &mFolderDescriptorSet;
				item.dirEntry = std::filesystem::directory_entry(parentPath);
				item.ext = "";
				item.texture = mFolderTexture;

				Clicked(item);
			}

			currentPos = { ImGui::GetCursorPos().x, ImGui::GetCursorPos().y };
			ImGui::SetCursorPos(currentPos);

			ImGui::Text("...");

			ImGui::EndGroup();
		}

		ImGui::SameLine();

		for (size_t i = 0; i < currDirItems.size(); i++)
		{
			// checks if must draw in the same line
			if ((currentPos.x + (buttonSize.x * 2)) <= ImGui::GetContentRegionAvail().x) ImGui::SameLine();

			// draws
			{
				ImGui::BeginGroup();

				if (ImGui::ImageButton(currDirItems[i].dirEntry.path().string().c_str(), *currDirItems[i].descriptor, buttonSize))
				{
					Clicked(currDirItems[i]);
				}

				currentPos = ImGui::GetCursorPos();
				ImGui::SetCursorPos({ currentPos.x + offSet, currentPos.y });

				ImGui::Text(currDirItems[i].dirEntry.path().filename().string().c_str());

				ImGui::EndGroup();
			}
		}

		ImGui::EndChild();
		ImGui::End();
	}

	void Explorer::OnDestroy()
	{
		for (auto& texture : mExtensionTexture)
		{
			delete texture;
		}

		delete mFolderTexture;
	}

	void Explorer::Clicked(ItemProperties& item)
	{
		if (item.dirEntry.is_directory())
		{
			mRoot = item.dirEntry.path().string();
		}
	}

	void Explorer::CreateTextures()
	{
		std::filesystem::path texturePath = mRoot;

		// folder
		{
			texturePath /= "textures";
			texturePath /= "editor";
			texturePath /= "folder.png";
			mFolderTexture = new VKTexture2D(mRenderer->BackendDevice(), texturePath.string().c_str());
			mFolderDescriptorSet = AddTexture(mFolderTexture->Sampler(), mFolderTexture->View(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		}

		// extensions
		{
			for (size_t i = 0; i < mExtensionTexturePaths.size(); i++)
			{
				texturePath = mRoot;
				texturePath /= "textures";
				texturePath /= "editor";
				texturePath /= mExtensionTexturePaths[i];
				mExtensionTexture[i] = new VKTexture2D(mRenderer->BackendDevice(), texturePath.string().c_str());
				mExtensionDescriptors[i] = AddTexture(mExtensionTexture[i]->Sampler(), mExtensionTexture[i]->View(), VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
			}
		}
	}

	std::vector<Explorer::ItemProperties> Explorer::RefreshExplorer(std::string root, bool onlyCurrentFolder)
	{
		std::filesystem::path currPath = mRoot;
		std::vector<std::filesystem::directory_entry> dirItems = {};
		std::vector<Explorer::ItemProperties> items = {};

		if (onlyCurrentFolder)
		{
			for (const std::filesystem::directory_entry& dirEntry : std::filesystem::directory_iterator(mRoot))
			{
				if (std::filesystem::is_directory(dirEntry))
				{
					ItemProperties item;
					item.dirEntry = dirEntry;
					item.ext = "";
					item.texture = mFolderTexture;
					item.descriptor = &mFolderDescriptorSet;
					items.push_back(item);

					dirItems.push_back(dirEntry);
					continue;
				}

				// file, check for valid extensions
				std::string fileExt = dirEntry.path().extension().string();
				for (size_t i = 0; i < mValidExtensions.size(); i++)
				{
					if (strcmp(mValidExtensions[i], fileExt.c_str()) == 0)
					{
						ItemProperties item;
						item.dirEntry = dirEntry;
						item.ext = mValidExtensions[i];
						item.texture = mExtensionTexture[i];
						item.descriptor = &mExtensionDescriptors[i];
						items.push_back(item);

						dirItems.push_back(dirEntry);
					}
				}
			}

			return items;
		}

		for (const std::filesystem::directory_entry& dirEntry : std::filesystem::recursive_directory_iterator(mRoot))
		{
			if (std::filesystem::is_directory(dirEntry))
			{
				ItemProperties item;
				item.dirEntry = dirEntry;
				item.ext = "";
				item.texture = mFolderTexture;
				item.descriptor = &mFolderDescriptorSet;
				items.push_back(item);

				dirItems.push_back(dirEntry);
				continue;
			}

			// file, check for valid extensions
			std::string fileExt = dirEntry.path().extension().string();
			for (size_t i = 0; i < mValidExtensions.size(); i++)
			{
				if (strcmp(mValidExtensions[i], fileExt.c_str()) == 0)
				{
					ItemProperties item;
					item.dirEntry = dirEntry;
					item.ext = mValidExtensions[i];
					item.texture = mExtensionTexture[i];
					item.descriptor = &mExtensionDescriptors[i];
					items.push_back(item);

					dirItems.push_back(dirEntry);
				}
			}
		}

		return items;
	}
}