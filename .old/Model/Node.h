#pragma once

#include "Vertex.h"
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