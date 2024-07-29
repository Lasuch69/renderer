#include <cmath>
#include <cstring>

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

	float data[4][4];
	data[0][0] = 1.0f / (aspect * tanHalfFovY);
	data[0][1] = 0.0f;
	data[0][2] = 0.0f;
	data[0][3] = 0.0f;

	data[1][0] = 0.0f;
	data[1][1] = 1.0f / tanHalfFovY;
	data[1][2] = 0.0f;
	data[1][3] = 0.0f;

	data[2][0] = 0.0f;
	data[2][1] = 0.0f;
	data[2][2] = -(zFar + zNear) / (zFar - zNear);
	data[2][3] = -1.0f;

	data[3][0] = 0.0f;
	data[3][1] = 0.0f;
	data[3][2] = -(2.0f * zFar * zNear) / (zFar - zNear);
	data[3][3] = 0.0f;

	mat4 out;
	memcpy(&out, data, sizeof(mat4));
	return REVERSE_Z_MATRIX * OPENGL_TO_VULKAN_MATRIX * out;
}

mat4 math::lookAt(const vec3 &eye, const vec3 &target, const vec3 &up) {
	vec3 f = normalize(target - eye);
	vec3 r = normalize(cross(up, f));
	vec3 u = cross(f, r);

	float data[4][4];
	data[0][0] = -r.x;
	data[0][1] = u.x;
	data[0][2] = -f.x;
	data[0][3] = 0.0;

	data[1][0] = r.y;
	data[1][1] = u.y;
	data[1][2] = -f.y;
	data[1][3] = 0.0;

	data[2][0] = -r.z;
	data[2][1] = u.z;
	data[2][2] = -f.z;
	data[2][3] = 0.0;

	data[3][0] = -dot(r, eye);
	data[3][1] = -dot(u, eye);
	data[3][2] = dot(f, eye);
	data[3][3] = 1.0;

	mat4 out;
	memcpy(&out, data, sizeof(mat4));
	return out;
}
