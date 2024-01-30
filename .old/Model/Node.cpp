#include "epch.h"
#include "Node.h"

#include "Skin.h"
#include "Mesh.h"

namespace Cosmos::ModelHelper
{
	Node::~Node()
	{
		if (mesh != nullptr)
			delete mesh;
			
		for (auto& child : children)
			delete child;
	}

	void Node::Update()
	{
		if (mesh != nullptr)
		{
			glm::mat4 m = GetFullMatrix();

			if (skin != nullptr)
			{
				// update joint matrices
				glm::mat4 inverseTransform = glm::inverse(m);
				size_t numJoints = std::min((uint32_t)skin->joints.size(), MAX_NUM_JOINTS);

				for (size_t i = 0; i < numJoints; i++)
				{
					Node* jointNode = skin->joints[i];
					glm::mat4 jointMat = jointNode->matrix * skin->inverseMatrices[i];
					jointMat = inverseTransform * jointMat;
					mesh->uniformBlock.jointMatrix[i] = jointMat;
				}

				mesh->uniformBlock.jointCount = (float)numJoints;
				memcpy(mesh->ubo.mapped, &mesh->ubo, sizeof(mesh->ubo));
			}

			else
			{
				memcpy(mesh->ubo.mapped, &m, sizeof(glm::mat4));
			}
		}

		// update child nodes
		for (auto& child : children)
			child->Update();
	}

	glm::mat4 Node::GetLocalMatrix()
	{
		return glm::translate(glm::mat4(1.0f), translation) * glm::mat4(rotation) * glm::scale(glm::mat4(1.0f), scale) * matrix;
	}

	glm::mat4 Node::GetFullMatrix()
	{
		glm::mat4 localMatrix = GetLocalMatrix();
		Node* p = parent;

		while (p != nullptr)
		{
			localMatrix = p->GetLocalMatrix() * localMatrix;
			p = p->parent;
		}

		return localMatrix;
	}
}