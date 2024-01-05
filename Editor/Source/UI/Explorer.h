#pragma once

#include <Engine.h>
#include <array>
#include <filesystem>

namespace Cosmos
{
	class Explorer : public Widget
	{
	public:

		struct ItemProperties
		{
			std::filesystem::directory_entry dirEntry;
			std::string ext;
			std::shared_ptr<Texture2D> texture = nullptr;
			VkDescriptorSet* descriptor = VK_NULL_HANDLE;
		};

	public:

		// constructor
		Explorer(std::shared_ptr<Renderer>& renderer);

		// destructor
		virtual ~Explorer() = default;

	public:

		// updates the ui element
		virtual void OnUpdate() override;

		// called for freeing resources
		virtual void OnDestroy() override;

	public:

		// clicked an item
		void Clicked(ItemProperties& item);

		// creates all textures used by the explorer tab
		void CreateTextures();

		// reloads the files the explorer have
		std::vector<ItemProperties> RefreshExplorer(std::string root, bool onlyCurrentFolder);

	private:

		std::shared_ptr<Renderer>& mRenderer;
		std::string mRoot = "Data";

		// folder 
		std::shared_ptr<Texture2D> mFolderTexture;
		VkDescriptorSet mFolderDescriptorSet;

		// extensions
		const std::array<const char*, 11> mValidExtensions =
		{
			".cfg", ".ini", ".txt",		// misc
			".gltf", ".glb",			// models
			".vert", ".frag", ".spv",	// shaders
			".png", ".jpg", ".ktx"		// textures
		};

		// extension pictures, must match with above vector
		const std::array<const char*, 11> mExtensionTexturePaths =
		{
			"cfg.png", "ini.png", "txt.png",		// misc
			"gltf.png", "glb.png",					// models
			"vert.png", "frag.png", "spv.png",		// shaders
			"png.png", "jpg.png", "ktx.png"			// textures
		};

		std::array<std::shared_ptr<Texture2D>, 11> mExtensionTexture;
		std::array<VkDescriptorSet, 11> mExtensionDescriptors;
	};
}