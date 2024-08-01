#include <cmath>

#include "types/mat4.h"
#include "types/vec3.h"

#include "projection.h"

using namespace math;

const mat4 OPENGL_TO_VULKAN_MATRIX = {
	{ 1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, -1.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, 0.5f, 0.0f },
	{ 0.0f, 0.0f, 0.5f, 1.0f },
};

const mat4 REVERSE_Z_MATRIX = {
	{ 1.0f, 0.0f, 0.0f, 0.0f },
	{ 0.0f, 1.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, -1.0f, 0.0f },
	{ 0.0f, 0.0f, 1.0f, 1.0f },
};

mat4 math::perspective(float aspect, float fovY, float zNear, float zFar) {
	float tanHalfFovY = std::tan(fovY / 2.0f);

	float x = 1.0f / (aspect * tanHalfFovY);
	float y = 1.0f / tanHalfFovY;

	float f = -(zFar + zNear) / (zFar - zNear);
	float n = -(2.0f * zFar * zNear) / (zFar - zNear);

	mat4 projection = {
		{ x, 0.0f, 0.0f, 0.0f },
		{ 0.0f, y, 0.0f, 0.0f },
		{ 0.0f, 0.0f, f, 0.0f },
		{ 0.0f, 0.0f, n, 0.0f },
	};

	return REVERSE_Z_MATRIX * OPENGL_TO_VULKAN_MATRIX * projection;
}

mat4 math::lookAt(const vec3 &eye, const vec3 &target, const vec3 &up) {
	vec3 f = normalize(target - eye);
	vec3 r = normalize(cross(up, f));
	vec3 u = cross(f, r);

	const vec3 p = { -dot(r, eye), -dot(u, eye), dot(f, eye) };

	return {
		{ -r.x, u.x, -f.x, 0.0f },
		{ r.y, u.y, f.y, 0.0f },
		{ -r.z, u.z, -f.z, 0.0f },
		{ p.x, p.y, p.z, 1.0f },
	};
}
