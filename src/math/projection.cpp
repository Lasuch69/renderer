#include <cmath>

#include "projection.h"

static vec3 cross(const vec3 &a, const vec3 &b) {
	return {
		(a.y * b.z) - (a.z * b.y),
		(a.z * b.x) - (a.x * b.z),
		(a.x * b.y) - (a.y * b.x),
	};
}

static float dot(const vec3 &a, const vec3 &b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

static vec3 normalize(const vec3 &v) {
	float denom = 1.0 / std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	return {
		v.x * denom,
		v.y * denom,
		v.z * denom,
	};
}

mat4 projectionMatrix(float aspect, float fovY, float zNear, float zFar) {
	float tanHalfFovY = std::tan(fovY / 2.0f);

	mat4 m;
	m.n[0][0] = 1.0f / (aspect * tanHalfFovY);
	m.n[1][1] = 1.0f / tanHalfFovY;
	m.n[2][2] = -(zFar + zNear) / (zFar - zNear);
	m.n[2][3] = -(2.0f * zFar * zNear) / (zFar - zNear);
	m.n[3][2] = -1.0f;
	m.n[3][3] = 0.0f;

	return m;
}

mat4 viewMatrix(vec3 eye, vec3 front) {
	vec3 f = normalize({ front.x - eye.x, front.y - eye.y, front.z - eye.z });
	vec3 r = normalize(cross(CAMERA_UP, f));
	vec3 u = cross(f, r);

	mat4 m;
	m.n[0][0] = r.x;
	m.n[0][1] = r.y;
	m.n[0][2] = r.z;
	m.n[0][3] = dot({ -r.x, -r.y, -r.z }, eye);

	m.n[1][0] = u.x;
	m.n[1][1] = u.y;
	m.n[1][2] = u.z;
	m.n[1][3] = dot({ -u.x, -u.y, -u.z }, eye);

	m.n[2][0] = -f.x;
	m.n[2][1] = -f.y;
	m.n[2][2] = -f.z;
	m.n[2][3] = dot(f, eye);

	m.n[3][0] = 0.0;
	m.n[3][1] = 0.0;
	m.n[3][2] = 0.0;
	m.n[3][3] = 1.0;

	return m;
}

mat4 modelMatrix(vec3 position) {
	mat4 m;
}
