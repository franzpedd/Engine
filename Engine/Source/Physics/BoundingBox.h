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
		BoundingBox(glm::vec3 min, glm::vec3 max);

		// destructor
		~BoundingBox();

		// returns the colision box dimension
		BoundingBox AABB(glm::mat4 mat);

		// returns a reference to the minimum value
		inline glm::vec3& GetMin() { return mMin; }

		// sets a new min value
		inline void SetMin(glm::vec3& value) { mMin = value; }

		// returns a reference to the maximum value
		inline glm::vec3& GetMax() { return mMax; }

		// sets a new max value
		inline void SetMax(glm::vec3& value) { mMax = value; }

		// returns either if the bounding box is valid or not
		inline bool IsValid() { return mValid; }

		// sets the bounding box to valid or not
		inline void SetValid(bool value) { mValid = value; }

	private:

		glm::vec3 mMin;
		glm::vec3 mMax;
		bool mValid = false;

	};
}