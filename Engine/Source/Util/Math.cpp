#include "Math.h"

namespace Cosmos::math
{
	float Length(const Vec4 v0)
	{
		return glm::length(v0);
	}

	Vec3 Min(Vec3& v0, Vec3& v1)
	{
		return glm::min(v0, v1);
	}

	Vec3 Max(Vec3& v0, Vec3& v1)
	{
		return glm::max(v0, v1);
	}

	Mat4 Translate(Mat4& m0, Vec3& v0)
	{
		return glm::translate(m0, v0);
	}

	Mat4 Scale(Mat4& m0, Vec3& v0)
	{
		return glm::scale(m0, v0);
	}

	Mat4 Inverse(Mat4& m0)
	{
		return glm::inverse(m0);
	}

	Vec3 Normalize(const Vec3& v0)
	{
		return glm::normalize(v0);
	}

	Quat Normalize(const Quat& q0)
	{
		return glm::normalize(q0);
	}

	Vec4 Mix(const Vec4& v0, const Vec4& v1, float f0)
	{
		return glm::mix(v0, v1, f0);
	}
}