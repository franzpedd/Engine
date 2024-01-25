#pragma once

#include "Util/Math.h"
#include <string>
#include <vector>

namespace Cosmos::ModelHelper
{
	// forward declaration
	struct Node;

	struct Skin
	{
	public:

		// constructor
		Skin() = default;

		// destructor
		~Skin() = default;

	public:

		std::string name = {};
		Node* root = nullptr;
		std::vector<Node*> joints = {};
		std::vector<glm::mat4> inverseMatrices = {};
	};
}