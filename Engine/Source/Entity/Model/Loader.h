#pragma once

#include "Animation.h"
#include "Mesh.h"
#include "Node.h"
#include "Primitive.h"
#include "Skin.h"
#include "Vertex.h"

#include "Renderer/Device.h"
#include "Renderer/Material.h"
#include "Renderer/Texture.h"

namespace Cosmos::ModelHelper
{
	struct Loader
	{
	public:

		struct Info
		{
			uint32_t* indexBuffer = 0;
			Vertex* vertexBuffer = nullptr;
			size_t indexPosition = 0;
			size_t vertexPosition = 0;
		};

	public:

		// constructor
		Loader(std::shared_ptr<Device>& device, std::string& path);

		// destructor
		~Loader() = default;

	public:

		// free used resources
		void Destroy();

		// loads model textures samplers and textures from tinygltf samplers
		void LoadTextures();

		// loads model materials from tinygltf materials
		void LoadMaterials();

		// loads model nodes from tinygltf nodes (must come after LoadMaterials as node's may have it's mesh with a material)
		void LoadNodes(Info& info, size_t& vertexCount, size_t& indexCount);

		// loads model animations from tinygltf animations
		void LoadAnimations();

		// loads model skins from tinygltf skins
		void LoadSkins();

		// create renderer resources
		void CreateRendererResources(Info& info, size_t& vertexCount, size_t& indexCount);

	public:

		// creates a model node from the tinygltf node
		void CreateNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, Info& info);

		// returns a ptr to a node after searching parent's children
		Node* FindChildNode(Node* parent, int32_t index);

		// returns a ptr to a node given it's index
		Node* FindNodeByIndex(int32_t index);

		// checks how many vertex and index exsts for a node
		void GetNodeProperties(tinygltf::Model& model, tinygltf::Node& node, size_t& vertexCount, size_t& indexCount);

	public:

		std::shared_ptr<Device>& device;
		std::string& path;
		tinygltf::Model model;

		std::vector<TextureSampler> samplers = {};
		std::vector<std::shared_ptr<Texture2D>> textures = {};
		std::vector<Material> materials = {};
		std::vector<Node*> linearNodes = {};
		std::vector<Node*> nodes = {};
		std::vector<Animation> animations = {};
		std::vector<Skin*> skins = {};

		// renderer resources
		VkBuffer vertexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory vertexMemory = VK_NULL_HANDLE;
		VkBuffer indexBuffer = VK_NULL_HANDLE;
		VkDeviceMemory indexMemory = VK_NULL_HANDLE;
	};
}