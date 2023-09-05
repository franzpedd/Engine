#pragma once

// todo: encapsulate glm into custom syntax to facilitate transition in the future
// also, visual studio stupidity

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26495 26451)
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

namespace Cosmos::math
{
	// wrap math library
	typedef glm::vec2 Vec2;
	typedef glm::vec3 Vec3;
	typedef glm::vec4 Vec4;

	typedef glm::mat2 Mat2;
	typedef glm::mat3 Mat3;
	typedef glm::mat4 Mat4;

	typedef glm::quat Quat;

	// returns a vector of 2 points of T
	template<typename T>
	inline Vec2 MakeVec2(const  T* ptr) { return glm::make_vec2(ptr); }

	// returns a vector of 3 points of T
	template<typename T>
	inline Vec3 MakeVec3(const T* ptr) { return glm::make_vec3(ptr); }

	// returns a vector of 4 points of T
	template<typename T>
	inline Vec4 MakeVec4(const T* ptr) { return glm::make_vec4(ptr); };
	
	// returns a 4x4 matrix of T
	template<typename T>
	inline Mat4 MakeMat4x4(const T* ptr) { return glm::make_mat4x4(ptr); }

	// returns a quaternion of T
	template<typename T>
	inline Quat MakeQuat(const T* ptr) { return glm::make_quat(ptr); }

	// returns the length of a vector
	float Length(const Vec4 v0);

	// returns v1 if v1 < v0. returns v0 otherwise
	Vec3 Min(Vec3& v0, Vec3& v1);

	// returns v1 if v1 < v0. returns v0 otherwise
	Vec3 Max(Vec3& v0, Vec3& v1);

	// translates the matrix by the vector and returns the new matrix
	Mat4 Translate(Mat4& m0, Vec3& v0);

	// scales the matrix by the vector and returns the new matrix
	Mat4 Scale(Mat4& m0, Vec3& v0);

	// calculates and returns the matrix inverse
	Mat4 Inverse(Mat4& m0);

	// normalizes the vector and returns it
	Vec3 Normalize(const Vec3& v0);

	// normalizes the quaternion and returns it
	Quat Normalize(const Quat& q0);

	// spherical linear interpolation of two quaternions
	Vec4 Mix(const Vec4& v0, const Vec4& v1, float f0);
}
