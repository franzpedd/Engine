#pragma once

#include <Engine.h>

#define MAX_SEARCH_PATH_SIZE 256
#define MAX_CHARS_TEXTURE_NAME 16

namespace Cosmos
{
	class TextureBrowser : public Widget
	{
	public:

		enum Extension
		{
			UNDEFINED = -1,
			JPG,
			PNG,
			TGA,
			BMP,

			EXTENSION_MAX
		};

		struct TextureProperties
		{
			std::shared_ptr<Texture2D> texture;
			std::filesystem::path path;
			Extension type = UNDEFINED;
			VkDescriptorSet descriptor = VK_NULL_HANDLE;
		};

	public:

		// constructure
		TextureBrowser(std::shared_ptr<Renderer>& renderer);

		// destructor
		~TextureBrowser();

		// for user interface drawing
		virtual void OnUpdate() override;

	private:

		// searchs for textures and returns a vector of valid ones
		std::vector<TextureProperties> SearchTextures(std::string str);

		// free current textures, searches for textures in the path
		void RefreshFilesExplorer(std::string path);

	private:

		std::shared_ptr<Renderer>& mRenderer;
		std::string mRoot = "Data";
		std::string mSearchStr;
		bool mUpdatedStr = false;
		const std::array<const char*, EXTENSION_MAX> mValidExtensions = { ".jpg", ".png", ".tga", ".bmp" };
		uint32_t mFoundTextures = 0;
		ImVec2 mTextureSize = { 128.0f, 128.0f };

		std::vector<TextureProperties> mTextures;
	};
}