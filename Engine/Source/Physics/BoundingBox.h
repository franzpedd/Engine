#pragma once

#include "Util/Math.h"

namespace Cosmos
{
	class BoundingBox
	{
	public:

		// constructor
		BoundingBox();

		// constructor with params
		BoundingBox(math::Vec3 min, math::Vec3 max);

		// destructor
		~BoundingBox();

		// returns the colision box dimension
		BoundingBox AABB(math::Mat4 mat);

		// returns a reference to the minimum value
		inline math::Vec3& GetMin() { return mMin; }

		// sets a new min value
		inline void SetMin(math::Vec3& value) { mMin = value; }

		// returns a reference to the maximum value
		inline math::Vec3& GetMax() { return mMax; }

		// sets a new max value
		inline void SetMax(math::Vec3& value) { mMax = value; }

		// returns either if the bounding box is valid or not
		inline bool IsValid() { return mValid; }

		// sets the bounding box to valid or not
		inline void SetValid(bool value) { mValid = value; }

	private:

		math::Vec3 mMin;
		math::Vec3 mMax;
		bool mValid = false;

	};
}