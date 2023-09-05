#include "BoundingBox.h"

namespace Cosmos
{
	BoundingBox::BoundingBox()
	{
	}

	BoundingBox::BoundingBox(math::Vec3 min, math::Vec3 max)
		: mMin(min), mMax(max)
	{
	}

	BoundingBox::~BoundingBox()
	{
	}

	BoundingBox BoundingBox::AABB(math::Mat4 mat)
	{
		math::Vec3 min = math::Vec3(mat[3]);
		math::Vec3 max = min;
		math::Vec3 v0;
		math::Vec3 v1;

		// calculate side
		math::Vec3 side = math::Vec3(mat[0]);
		v0 = side * mMin.x;
		v1 = side * mMax.x;
		min += math::Min(v0, v1);
		max += math::Max(v0, v1);

		// calculate up
		math::Vec3 up = math::Vec3(mat[1]);
		v0 = up * mMin.y;
		v1 = up * mMax.y;
		min += math::Min(v0, v1);
		max += math::Max(v0, v1);

		// calculate back
		math::Vec3 back = math::Vec3(mat[2]);
		v0 = back * mMin.z;
		v1 = back * mMax.z;
		min += math::Min(v0, v1);
		max += math::Max(v0, v1);

		return BoundingBox(min, max);
	}
}