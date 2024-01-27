#include "epch.h"
#include "Mesh.h"

#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VKBuffer.h"

namespace Cosmos::ModelHelper
{
	Mesh::Mesh(std::shared_ptr<Renderer>& renderer, glm::mat4 matrix)
		: renderer(renderer)
	{
		uniformBlock.matrix = matrix;

		VK_ASSERT
		(
			BufferCreate
			(
				renderer->GetDevice(),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				sizeof(uniformBlock),
				&ubo.buffer,
				&ubo.memory,
				&uniformBlock
			),
			"Failed to create Mesh Buffer"
		);

		VK_ASSERT
		(
			vkMapMemory
			(
				renderer->GetDevice()->GetDevice(),
				ubo.memory,
				0,
				sizeof(uniformBlock),
				0,
				&ubo.mapped
			),
			"Failed to map Mesh uniform block object memory"
		);

		ubo.descriptor = { ubo.buffer, 0, sizeof(ubo) };
	}

	Mesh::~Mesh()
	{
		vkDestroyBuffer(renderer->GetDevice()->GetDevice(), ubo.buffer, nullptr);
		vkFreeMemory(renderer->GetDevice()->GetDevice(), ubo.memory, nullptr);

		for (Primitive* primitive : primitives)
			delete primitive;
	}

	void Mesh::SetBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.SetMin(min);
		bb.SetMax(max);
		bb.SetValid(true);
	}
}