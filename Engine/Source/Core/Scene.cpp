#include "Scene.h"

#include "Util/Logger.h"

namespace Cosmos
{
	std::shared_ptr<Scene> Scene::Create(std::shared_ptr<Platform>& platform, std::shared_ptr<VKRenderer>& renderer)
	{
		return std::make_shared<Scene>(platform, renderer);
	}

	Scene::Scene(std::shared_ptr<Platform>& platform, std::shared_ptr<VKRenderer>& renderer)
		: m_Platform(platform), m_Renderer(renderer)
	{
		LOG_TRACE("Creating VKScene");

		m_Paths.Assets = std::filesystem::current_path().string();
		m_Paths.Assets /= "Data";

		m_Paths.Environments = std::filesystem::current_path().string();
		m_Paths.Environments /= "Data";
		m_Paths.Environments /= "Environments";

		m_Paths.Models = std::filesystem::current_path().string();
		m_Paths.Models /= "Data";
		m_Paths.Models /= "Models";

		m_Paths.Shaders = std::filesystem::current_path().string();
		m_Paths.Shaders /= "Data";
		m_Paths.Shaders /= "Shaders";

		m_Paths.Textures = std::filesystem::current_path().string();
		m_Paths.Textures /= "Data";
		m_Paths.Textures /= "Textures";
	}

	Scene::~Scene()
	{
	}

	void Scene::Prepare()
	{
		m_Camera.GetSpecification().Type = Camera::Type::LOOK_AT;
		m_Camera.GetSpecification().RotationSpeed = 0.25f;
		m_Camera.GetSpecification().MovementSpeed = 0.1f;
		m_Camera.SetPespective(45.0f, (f32)m_Platform->GetSize().first / (f32)m_Platform->GetSize().second, 0.1f, 256.0f);
		m_Camera.SetPosition({ 0.0f, 0.0f, 1.0f });
		m_Camera.SetRotation({ 0.0f, 0.0f, 0.0f });

		// 
		LoadTestScene();
		//GenerateBRDFLUT();
		//PrepareUniformBuffers();
		//SetupDescriptors();
		//PreparePipelines();

		// create UI
		// update overlay
		// record commandbuffer
	}

	void Scene::LoadTestScene()
	{
		// load environment
		//ReadDirectory(m_Paths.Environments.string(), "*.ktx", m_Environments, false);

		m_Textures.Empty = std::make_shared<VKTexture2D>(m_Renderer->GetDevice());

		std::filesystem::path samplePath = m_Paths.Textures;
		samplePath /= "empty.ktx";
		m_Textures.Empty->LoadFromFile(samplePath.string(), VK_FORMAT_R8G8B8A8_UNORM, m_Renderer->GetDevice()->GetSpecification().Queue);

		std::string sceneFile = m_Paths.Models.string() + "/DamagedHelmet/glTF-Embedded/DamagedHelmet.gltf";
		std::string envFile = m_Paths.Environments.string() + "papermill.ktx";


		//m_Textures.Empty.LoadFromFile();
	}
}