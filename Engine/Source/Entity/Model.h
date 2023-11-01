#pragma once

#include "Entity.h"
#include "Physics/BoundingBox.h"
#include "Util/Math.h"

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26495)
#endif
#include <tiny_gltf.h>
#if defined(_MSC_VER)
#pragma warning( pop)
#endif

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

#define MAX_NUM_JOINTS 128u

namespace Cosmos
{
	// forward declarations
	class Renderer;
	class GLTFNode;

	class Texture
	{
	public:

		struct Sampler
		{
			VkFilter magFilter = VK_FILTER_NEAREST;
			VkFilter minFilter = VK_FILTER_NEAREST;
			VkSamplerAddressMode addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			VkSamplerAddressMode addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			VkSamplerAddressMode addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		};

	public:

		// constructor
		Texture(std::shared_ptr<Renderer>& renderer);

		// destructor
		~Texture();

		// updates the descriptor image info for later usage
		void UpdateDescriptor();

		// free used resouces for the image
		void Cleanup();

		// loads the texture out of a gltf image
		void LoadFromGLTFImage(tinygltf::Image& img, Sampler sampler);

	private:

		std::shared_ptr<Renderer>& mRenderer;

		VkImage mImage = VK_NULL_HANDLE;
		VkDeviceMemory mImageMemory = VK_NULL_HANDLE;
		VkImageLayout mImageLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		VkImageView mImageView = VK_NULL_HANDLE;
		VkSampler mSampler = VK_NULL_HANDLE;
		VkDescriptorImageInfo mDescriptor = {};

		uint32_t mWidth = 0;
		uint32_t mHeight = 0;
		uint32_t mLayerCount = 0;
		uint32_t mMipLevels = 0;
	};

	class Material
	{
	public:

		enum AlphaMode
		{
			ALPHAMODE_OPAQUE = 0,
			ALPHAMODE_MASK,
			ALPHAMODE_BLEND
		};

		struct TextureCoordinateSets
		{
			uint8_t baseColor = 0;
			uint8_t metallicRoughness = 0;
			uint8_t specularGlossiness = 0;
			uint8_t normal = 0;
			uint8_t occlusion = 0;
			uint8_t emissive = 0;
		};

		struct Extension // specular/glossiness extension (using metallicRoughness as default)
		{
			glm::vec4 specularFactor = glm::vec4(0.0f);
			Texture* specularGlossiness;

			glm::vec4 diffuseFactor = glm::vec4(1.0f);
			Texture* diffuse;
		};

		struct PBRWorkflow
		{
			bool metallicRoughness = true;
			bool specularGlossiness = false;
		};

	public:

		// constructor
		Material() = default;

		// destructor
		~Material() = default;

	public:

		// returns a reference to the alpha mode
		inline AlphaMode& Alpha() { return mAlphaMode; }
		
		// returns a reference to the alpha cut-off
		inline float& AlphaCutoff() { return mAlphaCutoff; }

		// returns a reference to the metallic factor
		inline float& MetallicFactor() { return mMetallicFactor; }

		// returns a reference to the roughness factor
		inline float& RoughnessFactor() { return mRoughnessFactor; }

		// returns a reference to the base color
		inline glm::vec4& BaseColor() { return mBaseColor; }

		// returns a reference to the emissive factor
		inline glm::vec4& EmissiveFactor() { return mEmissiveFactor; }

		// returns a reference to the base texture
		inline Texture*& BaseTexture() { return mBase; }

		// returns a reference to the metallic/roughness texture
		inline Texture*& MetallicRoughnessTexture() { return mMetallicRoughness; }

		// retutrns a reference to the normal texture
		inline Texture*& NormalTexture() { return mNormal; }

		// returns a reference to the occlusion texture
		inline Texture*& OcclusionTexture() { return mOcclusion; }

		// returns a reference to the emissive texture
		inline Texture*& EmissiveTexture() { return mEmissive; }

		// returns a reference to doublesized flag
		inline bool& DoubleSized() { return mDoubleSized; }

		// returns a reference to the texture coordinate sets
		inline TextureCoordinateSets& TexCoords() { return mTexCoordSets; }

		// returns a reference to the extensions
		inline Extension& Extensions() { return mExtension; }

		// returns a reference to the PBR workflow 
		inline PBRWorkflow& Workflow() { return mWorkflow; }

		// returns a reference to the used descriptor set
		inline VkDescriptorSet& DescriptorSet() { return mDescriptorSet; }

	private:

		AlphaMode mAlphaMode = AlphaMode::ALPHAMODE_OPAQUE;
		float mAlphaCutoff = 1.0f;
		float mMetallicFactor = 1.0f;
		float mRoughnessFactor = 1.0f;
		glm::vec4 mBaseColor = glm::vec4(1.0f);
		glm::vec4 mEmissiveFactor = glm::vec4(1.0f);
		Texture* mBase = nullptr;
		Texture* mMetallicRoughness = nullptr;
		Texture* mNormal = nullptr;
		Texture* mOcclusion = nullptr;
		Texture* mEmissive = nullptr;
		bool mDoubleSized = false;
		TextureCoordinateSets mTexCoordSets = {};
		Extension mExtension = {};
		PBRWorkflow mWorkflow = {};
		VkDescriptorSet mDescriptorSet = VK_NULL_HANDLE;
	};

	class Primitive
	{
	public:

		// constructor
		Primitive(Material& material, uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount);

		// destructor
		~Primitive() = default;

		// sets the primitive's bounding box
		void SetBoundingBox(glm::vec3 min, glm::vec3 max);

		// returns the primitive's bounding box
		inline BoundingBox& BB() { return mBB; }

		// returns a reference to the first index
		inline uint32_t& FirstIndex() { return mFirstIndex; }

		// returns a reference to the index count
		inline uint32_t& IndexCount() { return mIndexCount; }

		// returns a reference to the vertex count
		inline uint32_t VertexCount() { return mVertexCount; }

	private:

		Material& mMaterial;
		uint32_t mFirstIndex = 0;
		uint32_t mIndexCount = 0;
		uint32_t mVertexCount = 0;
		bool mHasIndices = false;
		BoundingBox mBB = {};
	};

	class Mesh
	{
	public:

		struct UBO
		{
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
			VkDescriptorBufferInfo descriptor = {};
			VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
			void* mapped = nullptr;
		};

		struct UniformBlock
		{
			glm::mat4 matrix = {};
			glm::mat4 jointMatrix[MAX_NUM_JOINTS] = {};
			float jointCount = 0.0f;
		};

	public:

		// constructor
		Mesh(std::shared_ptr<Renderer>& renderer, glm::mat4 matrix);

		// destructor
		~Mesh();

		// sets the meshe's bounding box
		void SetBoundingBox(glm::vec3 min, glm::vec3 max);

		// returns a reference to the uniform block
		inline UniformBlock& GetUniformBlock() { return mUniformBlock; }

		// returns a reference to the uniform buffer object
		inline UBO& GetUBO() { return mUBO; }

		// returns a reference to the primitives vector
		inline std::vector<Primitive*>& Primitives() { return mPrimitives; }

		// returns the primitive bounding box
		inline BoundingBox& BB() { return mBB; }

		// returns the primitive dimensions
		inline BoundingBox& AABB() { return mAABB; }

	private:

		std::shared_ptr<Renderer>& mRenderer;
		std::vector<Primitive*> mPrimitives = {};
		BoundingBox mBB = {};
		BoundingBox mAABB = {};
		UBO mUBO = {};
		UniformBlock mUniformBlock = {};
	};

	class Skin
	{
	public:

		// constructor
		Skin() = default;

		// destructor
		~Skin() = default;

		// returns a reference to skin's name
		inline std::string& Name() { return mName; }

		// returns a reference to skin's root
		inline GLTFNode*& Root() { return mRoot; }

		// returns a reference to the skin inverse matrices
		inline std::vector<glm::mat4>& InverseBindMatrices() { return mInverseBindMatrices; }

		// returns a reference to the vector of joints nodes
		inline std::vector<GLTFNode*>& JointNodes() { return mJoints; }

	private:

		std::string mName = {};
		GLTFNode* mRoot = nullptr;
		std::vector<glm::mat4> mInverseBindMatrices = {};
		std::vector<GLTFNode*> mJoints = {};
	};

	class GLTFNode
	{
	public:

		// constructor
		GLTFNode() = default;

		// destructor
		~GLTFNode();

		// updates the node
		void Update();

		// calculates and returns the node matrix
		glm::mat4 LocalMatrix();

		// returns the node matrix
		glm::mat4 GetMatrix();

	public:

		// returns a reference to the node name
		inline std::string& Name() { return mName; }

		// returns a reference to the pointer for the node's parent
		inline GLTFNode*& GetParent() { return mParent; }

		// returns a reference to the meshes skin's index
		inline int32_t& Index() { return mIndex; }

		// returns a reference to the vector of children nodes
		inline std::vector<GLTFNode*>& Children() { return mChildren; }

		// returns a reference to the node's matrix
		inline glm::mat4& Matrix() { return mMatrix; }

		// returns a reference to the pointer for the node's mesh
		inline Mesh*& GetMesh() { return mMesh; }

		// returns a reference to the pointer for the node's skin
		inline Skin*& GetSkin() { return mSkin; }

		// returns a reference to the meshes skin's index
		inline int32_t& SkinIndex() { return mSkinIndex; }

		// returns a reference to the node's translation vector
		inline glm::vec3& Translation() { return mTranslation; }

		// returns a reference to the node's scale vector
		inline glm::vec3& Scale() { return mScale; }

		// returns a reference to the rotation quaternion
		inline glm::quat Rotation() { return mRotation; }

		// returns a reference to the model's bvh
		inline BoundingBox& BVH() { return mBVH; }

		// returns a reference to the model's aabb
		inline BoundingBox& AABB() { return mAABB; }

	private:

		std::string mName = {};
		GLTFNode* mParent = nullptr;
		int32_t mIndex = 0;
		std::vector<GLTFNode*> mChildren = {};
		glm::mat4 mMatrix = {};
		Mesh* mMesh = nullptr;
		Skin* mSkin = nullptr;
		int32_t mSkinIndex = -1;
		glm::vec3 mTranslation = {};
		glm::vec3 mScale = {1.0f, 1.0f, 1.0f};
		glm::quat mRotation = {};
		BoundingBox mBVH = {};
		BoundingBox mAABB = {};
	};

	class Animation
	{
	public:

		struct Channel
		{
			enum PathType
			{
				PATH_TYPE_TRANSLATION = 0,
				PATH_TYPE_ROTATION,
				PATH_TYPE_SCALE
			};

			PathType pathType = PATH_TYPE_TRANSLATION;
			GLTFNode* node = nullptr;
			uint32_t samplerIndex = 0;
		};

		struct Sampler
		{
			enum InterpolationType
			{
				INTERPOLATION_TYPE_LINEAR = 0,
				INTERPOLATION_TYPE_STEP,
				INTERPOLATION_CUBIC_SPLINE
			};

			InterpolationType interpolationType = INTERPOLATION_TYPE_LINEAR;
			std::vector<float> inputs = {};
			std::vector<glm::vec4> outputs = {};
		};

	public:

		// constructor
		Animation() = default;

		// destructor
		~Animation() = default;

		// returns a reference to the animation's name
		inline std::string& Name() { return mName; }

		// returns a reference to the vector of channels
		inline std::vector<Channel>& Channels() { return mChannels; }

		// returns a reference to the vector of samplers
		inline std::vector<Sampler>& Samplers() { return mSamplers; }

		// returns the start value
		inline float& GetStart() { return mStart; }

		// sets a new start value
		inline void SetStart(float value) { mStart = value; }

		// returns the end value
		inline float& GetEnd() { return mEnd; }

		// sets a new start value
		inline void SetEnd(float value) { mEnd = value; }

	private:

		std::string mName = {};
		std::vector<Channel> mChannels = {};
		std::vector<Sampler> mSamplers = {};
		float mStart = std::numeric_limits<float>::max();
		float mEnd = std::numeric_limits<float>::min();
	};

	class Model : public Entity
	{
	public:

		struct Vertex
		{
			glm::vec3 pos = {};
			glm::vec3 normal = {};
			glm::vec2 uv0 = {};
			glm::vec2 uv1 = {};
			glm::vec4 joint0 = {};
			glm::vec4 weight0 = {};
			glm::vec4 color = {};
		};

		struct Vertices
		{
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
		};

		struct Indices
		{
			VkBuffer buffer = VK_NULL_HANDLE;
			VkDeviceMemory memory = VK_NULL_HANDLE;
		};

		struct Dimension
		{
			glm::vec3 min = glm::vec3(FLT_MIN);
			glm::vec3 max = glm::vec3(FLT_MAX);
		};

		struct LoaderInfo
		{
			uint32_t* indexBuffer = 0;
			Vertex* vertexBuffer = nullptr;
			size_t indexPos = 0;
			size_t vertexPos = 0;
		};

	public:

		// constructor
		Model(std::shared_ptr<Renderer>& renderer);

		// destructor
		virtual ~Model();

	public:

		// returns how many instances of the derivated object exists
		virtual uint64_t GetInstancesCount() override;

		// draws the entity (leave empty if doesnt required)
		virtual void OnRenderDraw() override;

		// updates the entity (leave empty if doesnt required)
		virtual void OnUpdate(float timestep) override;

		// called before destructor, for freeing resources
		virtual void OnDestroy() override;

	public:

		// draws a singular node
		void DrawNode(GLTFNode* node, VkCommandBuffer commandBuffer);

		// draws the model
		void Draw(VkCommandBuffer commandBuffer); 

		// updates the model animation cycle to the next step
		void UpdateAnimations(uint32_t index, float time);

	public:

		// loads a model from file path
		void LoadFromFile(std::string path, float scale = 1.0f);

		// cleans used resources for the model
		void Destroy();
		
		// loads a new gltf node based on previous GLTFNode
		void LoadNode(GLTFNode* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& mdl, LoaderInfo& loaderInfo, float globalScale);

		// loads gltf skins
		void LoadSkins(tinygltf::Model& mdl);

		// loads gltf textures
		void LoadTextures(tinygltf::Model& mdl);

		// loads gltf texture samplers // todo: merge with load textures
		void LoadTextureSamplers(tinygltf::Model& mdl);

		// loads gltf materials
		void LoadMaterials(tinygltf::Model& mdl);

		// loads gltf animations
		void LoadAnimations(tinygltf::Model& mdl);

		// fills-up the properties of a given node
		void NodeProperties(const tinygltf::Node& node, const tinygltf::Model& mdl, size_t& vertexCount, size_t& indexCount);

		// converts tinygltf wrap mode number to vulkan respective
		VkSamplerAddressMode WrapMode(int32_t mode);

		// converts tinygltf filter mode number to vulkan respective
		VkFilter FilterMode(int32_t mode);

		// calculates the bounding box of a node givent it's parent bounding box
		void CalculateBoundingBox(GLTFNode* node, GLTFNode* parent);

		// calculates the model dimensions
		void GetModelDimensions();

		// returns a node based on parent and index
		GLTFNode* FindNode(GLTFNode* parent, int32_t index);

		// returns a node based on index
		GLTFNode* NodeFromIndex(int32_t index);

	public: // to facilitate

		std::shared_ptr<Renderer>& mRenderer;
		Vertices mVertices = {};
		Indices mIndices = {};
		glm::mat4 mAABB = {};
		std::vector<GLTFNode*> mNodes = {};
		std::vector<GLTFNode*> mLinearNodes = {};
		std::vector<Skin*> mSkins = {};
		std::vector<Texture> mTextures = {};
		std::vector<Texture::Sampler> mTextureSamplers = {};  // todo: merge this to texture
		std::vector<Material> mMaterials = {};
		std::vector<Animation> mAnimations = {};
		std::vector<std::string> mExtensions = {};
		Dimension mDimension;
		LoaderInfo mLoaderInfo;

		// holds how many instances of cubes exists
		static uint64_t sInstances;
	};
}