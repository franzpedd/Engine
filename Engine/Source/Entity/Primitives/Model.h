#pragma once

#include "Defines.h"
#include "Physics/BoundingBox.h"
#include "Renderer/Texture.h"
#include "Renderer/Material.h"
#include "Util/Math.h"

#include <memory>

namespace Cosmos
{
	// forward declaration
	class Device;
	class ModelNode;

	class Model
	{
	private:

		struct Vertex
		{
			glm::vec3 position = {};
			glm::vec3 normal = {};
			glm::vec2 uv0 = {};
			glm::vec2 uv1 = {};
			glm::vec4 joint = {};
			glm::vec4 weight = {};
			glm::vec4 color = {};
		};

		struct LoaderInfo
		{
			uint32_t* indexBuffer = 0;
			Vertex* vertexBuffer = nullptr;
			size_t indexPosition = 0;
			size_t vertexPosition = 0;
		};

	public:

		// returns a smart-ptr to a new mesh
		static std::shared_ptr<Model> Create(std::shared_ptr<Device>& device);

		// constructor
		Model(std::shared_ptr<Device>& device);

		// destructor
		~Model();

	public:

		// loads a model from file path
		void LoadFromFile(std::string path);

	public:

		// return the model's path
		inline std::string GetPath() { return mPath; }

	private:

		// loads model's samplers
		void LoadSamplers(tinygltf::Model& model);

		// loads model's textures
		void LoadTextures(tinygltf::Model& model);

		// loads model's materials
		void LoadMaterials(tinygltf::Model& model);

		// loads model's animations
		void LoadAnimations(tinygltf::Model& model);

		// starts loading the model's nodes
		void LoadNodes(tinygltf::Model& model);

		// gets the properties (vertex and index count) of a given start node
		void GetNodeProperties(tinygltf::Model& model, tinygltf::Node& node, size_t& vertexCount, size_t& indexCount);

		// loads a gltf node and converts it to ModelNode
		void LoadNode(ModelNode* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, LoaderInfo& loaderInfo);

		// loads gltf node meshes 
		void LoadNodeMeshes(ModelNode* modelNode, const tinygltf::Node& node, const tinygltf::Model& model, LoaderInfo& info);

	private:

		std::shared_ptr<Device>& mDevice;
		std::string mPath = {};

		std::vector<TextureSampler> mSamplers = {};
		std::vector<std::shared_ptr<Texture2D>> mTextures = {};
		std::vector<Material> mMaterials = {};
	};

	// primitive
	class ModelPrimitive
	{
	public:

		// constructor
		ModelPrimitive(Material& material, uint32_t vertexCount, uint32_t indexCount, uint32_t firstIndex);

		// destructor
		~ModelPrimitive() = default;

		// returns the primitive dimension
		inline BoundingBox& GetBoundingBox() { return mBB; }

		// returns the how many vertices the primitive has
		inline uint32_t& GetVertexCount() { return mVertexCount; }

		// returns the how many indices the primitive has (0 if none)
		inline uint32_t& GetVertexCount() { return mVertexCount; }

		// returns the primitive's first index (0 if none)
		inline uint32_t& GetFirstIndex() { return mFirstIndex; }

	public:

		// sets the primitive's bounding box
		void SetBoundingBox(glm::vec3 min, glm::vec3 max);

	private:

		Material& mMaterial;
		uint32_t mVertexCount = 0;
		uint32_t mIndexCount = 0;
		uint32_t mFirstIndex = 0;
		bool mHasIndices = false;
		BoundingBox mBB = {};
	};

	class ModelSkin
	{
	public:

		// constructor
		ModelSkin();

		// destructor
		~ModelSkin();

	public:

		// returns a reference to skin's name
		inline std::string& GetName() { return mName; }

		// returns a reference to skin's root
		inline ModelNode*& GetRoot() { return mRoot; }

		// returns a reference to the skin inverse matrices
		inline std::vector<glm::mat4>& GetInverseMatrices() { return mInverseMatrces; }

		// returns a reference to the vector of joints nodes
		inline std::vector<ModelNode*>& GetJointNodes() { return mJoints; }

	private:

		std::string mName = {};
		ModelNode* mRoot = nullptr;
		std::vector<ModelNode*> mJoints = {};
		std::vector<glm::mat4> mInverseMatrces = {};
	};

	class ModelMesh
	{
	public:

		struct UniformBlock
		{
			glm::mat4 matrix = {};
			glm::mat4 jointMatrix[MAX_NUM_JOINTS] = {};
			float jointCount = 0.0f;
		};

		struct UBO
		{
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			VkDescriptorBufferInfo descriptor = {};
			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
			void* mapped = nullptr;
		};

	public:

		// constructor
		ModelMesh(std::shared_ptr<Device>& device, glm::mat4 matrix);

		// destructor
		~ModelMesh();

		// sets the meshe's bounding box
		void SetBoundingBox(glm::vec3 min, glm::vec3 max);

	public:

		// returns a reference to the uniform block
		inline UniformBlock& GetUniformBlock() { return mUniformBlock; }

		// returns a reference to the uniform buffer object
		inline UBO& GetUBO() { return mUBO; }

		// returns a reference to the primitives vector
		inline std::vector<ModelPrimitive*>& GetPrimitives() { return mPrimitives; }

		// returns the primitive bounding box
		inline BoundingBox& GetBB() { return mBB; }

		// returns the primitive dimensions
		inline BoundingBox& GetAABB() { return mAABB; }

	private:

		std::shared_ptr<Device>& mDevice;
		std::vector<ModelPrimitive*> mPrimitives = {};
		BoundingBox mBB = {};
		BoundingBox mAABB = {};
		UBO mUBO = {};
		UniformBlock mUniformBlock = {};
	};

	class ModelNode
	{
	public:

		// constructor
		ModelNode() = default;

		// destructor
		~ModelNode();

	public:

		// tick updates the node
		void Update();

		// calculates and returns the local node's matrix
		glm::mat4 GetLocalMatrix();

		// calculates and returns the node's (and parents nodes) matrix
		glm::mat4 GetCompleteMatrix();

	public:

		// returns a reference to the node name
		inline std::string& GetName() { return mName; }

		// returns a reference to the pointer for the node's parent
		inline ModelNode*& GetParent() { return mParent; }

		// returns a reference to the meshes skin's index
		inline int32_t& GetIndex() { return mIndex; }

		// returns a reference to the vector of children nodes
		inline std::vector<ModelNode*>& GetChildren() { return mChildren; }

		// returns a reference to the node's matrix
		inline glm::mat4& GetMatrix() { return mMatrix; }

		// returns a reference to the pointer for the node's mesh
		inline ModelMesh*& GetMesh() { return mMesh; }

		// returns a reference to the pointer for the node's skin
		inline ModelSkin*& GetSkin() { return mSkin; }

		// returns a reference to the meshes skin's index
		inline int32_t& GetSkinIndex() { return mSkinIndex; }

		// returns a reference to the node's translation vector
		inline glm::vec3& GetTranslation() { return mTranslation; }

		// returns a reference to the node's scale vector
		inline glm::vec3& GetScale() { return mScale; }

		// returns a reference to the rotation quaternion
		inline glm::quat GetRotation() { return mRotation; }

		// returns a reference to the model's bvh
		inline BoundingBox& GetBVH() { return mBVH; }

		// returns a reference to the model's aabb
		inline BoundingBox& GetAABB() { return mAABB; }

	private:

		std::string mName = {};
		ModelNode* mParent = nullptr;
		int32_t mIndex = 0;
		std::vector<ModelNode*> mChildren = {};
		glm::mat4 mMatrix = {};
		ModelMesh* mMesh = nullptr;
		ModelSkin* mSkin = nullptr;
		int32_t mSkinIndex = -1;
		glm::vec3 mTranslation = {};
		glm::vec3 mScale = { 1.0f, 1.0f, 1.0f };
		glm::quat mRotation = {};
		BoundingBox mBVH = {};
		BoundingBox mAABB = {};
	};
}