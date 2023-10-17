#include "BoundingBox.h"

namespace Cosmos
{
	BoundingBox::BoundingBox()
	{
	}

	BoundingBox::BoundingBox(glm::vec3 min, glm::vec3 max)
		: mMin(min), mMax(max)
	{
	}

	BoundingBox::~BoundingBox()
	{
	}

	BoundingBox BoundingBox::AABB(glm::mat4 mat)
	{
		glm::vec3 min = glm::vec3(mat[3]);
		glm::vec3 max = min;
		glm::vec3 v0;
		glm::vec3 v1;

		// calculate side
		glm::vec3 side = glm::vec3(mat[0]);
		v0 = side * mMin.x;
		v1 = side * mMax.x;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		// calculate up
		glm::vec3 up = glm::vec3(mat[1]);
		v0 = up * mMin.y;
		v1 = up * mMax.y;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		// calculate back
		glm::vec3 back = glm::vec3(mat[2]);
		v0 = back * mMin.z;
		v1 = back * mMax.z;
		min += glm::min(v0, v1);
		max += glm::max(v0, v1);

		return BoundingBox(min, max);
	}
}