#pragma once

#include "Physics/BoundingBox.h"
#include "Util/Math.h"

namespace Cosmos::ModelHelper
{
	// forward declaration
	struct Mesh;
	struct Skin;

	struct Node
	{
	public:

		// constructor
		Node() = default;

		// destructor
		~Node();

	public:

		// updates the node
		void Update();

		// calcualtes and returns node's matrix
		glm::mat4 GetLocalMatrix();

		// calculates and returns node's and node's parents matrix
		glm::mat4 GetFullMatrix();

	public:

		std::string name = {};
		Node* parent = nullptr;
		int32_t index = 0;
		std::vector<Node*> children = {};
		glm::mat4 matrix = {};
		Mesh* mesh = nullptr;
		Skin* skin = nullptr;
		int32_t skinIndex = -1;
		glm::vec3 translation = {};
		glm::vec3 scale = { 1.0f, 1.0f, 1.0f };
		glm::quat rotation = {};
		BoundingBox bvh = {};
		BoundingBox aabb = {};
	};
}

/*
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


*/