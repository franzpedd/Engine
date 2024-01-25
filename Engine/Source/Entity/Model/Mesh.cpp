#include "epch.h"
#include "Mesh.h"

#include "Primitive.h"
#include "Renderer/Device.h"
#include "Renderer/Vulkan/VKBuffer.h"

namespace Cosmos::ModelHelper
{
	Mesh::Mesh(std::shared_ptr<Device>& device, glm::mat4 matrix)
		: device(device)
	{
		uniformBlock.matrix = matrix;

		VK_ASSERT
		(
			BufferCreate
			(
				std::reinterpret_pointer_cast<VKDevice>(device),
				VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				sizeof(UniformBlock),
				&ubo.buffer,
				&ubo.memory,
				&uniformBlock
			), 
			"Failed to create buffer when creating a Mesh"
		);
	}

	Mesh::~Mesh()
	{
		vkDestroyBuffer(device->GetDevice(), ubo.buffer, nullptr);
		vkFreeMemory(device->GetDevice(), ubo.memory, nullptr);
		
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