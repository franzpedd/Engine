#pragma once

#include "Physics/BoundingBox.h"
#include "Renderer/Material.h"

namespace Cosmos::ModelHelper
{
	struct Primitive
	{
	public:

		// constructor
		Primitive(Material& material, uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount)
			: material(material)
		{
			hasIndices = indexCount > 0 ? true : false;
		}

		// destructor
		~Primitive() = default;

	public:

		// sets primitive's bounding box
		inline void SetBoundingBox(glm::vec3 min, glm::vec3 max)
		{
			bb.SetMin(min);
			bb.SetMax(max);
			bb.SetValid(true);
		}

	public:

		Material& material;
		uint32_t firstIndex = 0;
		uint32_t indexCount = 0;
		uint32_t vertexCount = 0;
		bool hasIndices = false;
		BoundingBox bb = {};
	};
}