#pragma once

#include "Core/Defines.h"

#include "Entity/Camera.h"

#include "Renderer/Vulkan/VKRenderer.h"
#include "Renderer/Vulkan/VKTexture.h"

#include <memory>
#include <filesystem>

namespace Cosmos
{
	class COSMOS_API Scene
	{
	public:

		struct Paths
		{
			std::filesystem::path Assets;
			std::filesystem::path Environments;
			std::filesystem::path Models;
			std::filesystem::path Shaders;
			std::filesystem::path Textures;
		};

		// this will be moved to an asset manager
		struct Textures
		{
			std::shared_ptr<VKTextureCubemap> Irradiance;
			std::shared_ptr<VKTextureCubemap> PrefilteredCube;
			std::shared_ptr<VKTextureCubemap> Environment;
			std::shared_ptr<VKTexture2D> Empty;
			std::shared_ptr<VKTexture2D> BRDF;
		};

	public:

		// returns a smart-ptr to a new scene
		static std::shared_ptr<Scene> Create(std::shared_ptr<Platform>& platform, std::shared_ptr<VKRenderer>& renderer);

		// constructor
		Scene(std::shared_ptr<Platform>& platform, std::shared_ptr<VKRenderer>& renderer);

		// destructor
		~Scene();

	public:

		// prepares to draw the scene
		void Prepare();

	private:

		// loads testing scene
		void LoadTestScene();

	private:

		std::shared_ptr<Platform>& m_Platform;
		std::shared_ptr<VKRenderer>& m_Renderer;
		Camera m_Camera;
		Paths m_Paths;

		// move this to asset manager
		std::map<std::string, std::string> m_Environments;
		std::string m_SelectedEnvironment = "papermill";
		Textures m_Textures;
	};
}