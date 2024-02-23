#pragma once

#include <Engine.h>
#include <array>
#include <filesystem>
#include <string>

#define EXPLORER_ASSET_NAME_MAX_DISPLAY_SIZE 10

namespace Cosmos
{
	class Explorer : public Widget
	{
	public:

		typedef enum AssetType
		{
			Undefined = 0,
			Folder,
			Text,						// text files
			Shader,						// shaders
			Image,						// images
			Model						// geometry
		} AssetType;

		typedef struct AssetResource
		{
			std::shared_ptr<Texture2D> texture = {};
			VkDescriptorSet descriptor = VK_NULL_HANDLE;
		} AssetResource;

		typedef struct Asset
		{
			AssetType type = AssetType::Undefined;
			AssetResource resource = {};
			std::string path = {};
			std::string displayName = {};
		} Asset;

	public:

		// constructor
		Explorer(std::shared_ptr<Renderer>& renderer);

		// destructor
		virtual ~Explorer();

	public:

		// updates the ui element
		virtual void OnUpdate() override;

	private:

		// reads the current directory and returns it's contents
		void ReadFolder(std::string path);

	private:

		std::shared_ptr<Renderer>& mRenderer;

		// explorer directory
		std::string mCurrentDir = {};
		std::vector<Asset> mCurrentDirAssets = {};
		bool mCurrentDirRefresh = true;

		// default resources
		AssetResource mUndefinedResource = {};
		AssetResource mFolderResource = {};
		AssetResource mTextResource = {};
		AssetResource mModelResource = {};
		AssetResource mVertexResource = {};
		AssetResource mFragmentResource = {};
		AssetResource mSpirvResource = {};
		//AssetResource mTextureResource = {}; // not used since textures are themselves rendered

		const std::array<const char*, 8> mValidExtensions =
		{
			".txt",							// text
			".gltf", ".obj",				// models
			".vert", ".frag", ".spv",		// shaders
			".png", ".jpg",					// textures
		};
	};
}