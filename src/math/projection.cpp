#include <cmath>
#include <cstring>

#include "projection.h"

using namespace math;

mat4 math::perspective(float aspect, float fovY, float zNear, float zFar) {
	float tanHalfFovY = std::tan(fovY / 2.0f);

	float data[4][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
	};

	data[0][0] = 1.0f / (aspect * tanHalfFovY);
	data[1][1] = 1.0f / tanHalfFovY;
	data[2][2] = -(zFar + zNear) / (zFar - zNear);
	data[2][3] = -(2.0f * zFar * zNear) / (zFar - zNear);
	data[3][2] = -1.0f;
	data[3][3] = 0.0f;

	mat4 out;
	memcpy(&out, data, sizeof(mat4));
	return out;
}

mat4 math::lookAt(const vec3 &eye, const vec3 &front, const vec3 &up) {
	vec3 f = normalize(front - eye);
	vec3 r = normalize(cross(up, f));
	vec3 u = cross(f, r);

	float data[4][4] = {
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 0.0f },
	};

	data[0][0] = r.x;
	data[0][1] = r.y;
	data[0][2] = r.z;
	data[0][3] = dot(-r, eye);

	data[1][0] = u.x;
	data[1][1] = u.y;
	data[1][2] = u.z;
	data[1][3] = dot(-u, eye);

	data[2][0] = -f.x;
	data[2][1] = -f.y;
	data[2][2] = -f.z;
	data[2][3] = dot(f, eye);

	data[3][0] = 0.0;
	data[3][1] = 0.0;
	data[3][2] = 0.0;
	data[3][3] = 1.0;

	mat4 out;
	memcpy(&out, data, sizeof(mat4));
	return out;
}
