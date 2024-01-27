#pragma once

#include "Physics/BoundingBox.h"
#include "Util/Math.h"

namespace Cosmos::ModelHelper
{
	// forward declaration
	struct Mesh;
	struct Skin;

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