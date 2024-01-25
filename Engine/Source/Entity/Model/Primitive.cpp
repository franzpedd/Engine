#include "epch.h"
#include "Primitive.h"

namespace Cosmos::ModelHelper
{
	Primitive::Primitive(Material& material, uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount)
		: material(material)
	{
		hasIndices = indexCount > 0 ? true : false;
	}

	void Primitive::SetBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		bb.SetMin(min);
		bb.SetMax(max);
		bb.SetValid(true);
	}
}