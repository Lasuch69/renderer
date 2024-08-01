#ifndef MATH_VEC3_H
#define MATH_VEC3_H

#include <cmath>

namespace math {

struct vec3 {
	float x, y, z;

	inline vec3 operator-() const {
		return { -x, -y, -z };
	}

	inline vec3 operator*(float v) const {
		return { x * v, y * v, z * v };
	}

	inline vec3 operator/(float v) const {
		return { x / v, y / v, z / v };
	}

	inline vec3 operator+(const vec3 &v) const {
		return { x + v.x, y + v.y, z + v.z };
	}

	inline vec3 operator-(const vec3 &v) const {
		return { x - v.x, y - v.y, z - v.z };
	}

	inline vec3 operator*(const vec3 &v) const {
		return { x * v.x, y * v.y, z * v.z };
	}

	inline vec3 operator/(const vec3 &v) const {
		return { x / v.x, y / v.y, z / v.z };
	}

	inline void operator*=(float v) {
		x *= v;
		y *= v;
		z *= v;
	}

	inline void operator/=(float v) {
		x /= v;
		y /= v;
		z /= v;
	}

	inline void operator+=(const vec3 &v) {
		x += v.x;
		y += v.y;
		z += v.z;
	}

	inline void operator-=(const vec3 &v) {
		x -= v.x;
		y -= v.y;
		z -= v.z;
	}

	inline void operator*=(const vec3 &v) {
		x *= v.x;
		y *= v.y;
		z *= v.z;
	}

	inline void operator/=(const vec3 &v) {
		x /= v.x;
		y /= v.y;
		z /= v.z;
	}

	vec3() {}

	vec3(float _v) {
		x = _v;
		y = _v;
		z = _v;
	}

	vec3(float _x, float _y, float _z) {
		x = _x;
		y = _y;
		z = _z;
	}
};

inline vec3 cross(const vec3 &a, const vec3 &b) {
	return { (a.y * b.z) - (a.z * b.y), (a.z * b.x) - (a.x * b.z), (a.x * b.y) - (a.y * b.x) };
}

inline float distance(const vec3 &a, const vec3 &b) {
	vec3 v = a - b;
	return std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

inline float dot(const vec3 &a, const vec3 &b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
}

inline float length(const vec3 &v) {
	return std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
}

inline vec3 normalize(const vec3 &v) {
	float denom = 1.0 / std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z));
	return v * denom;
}

} // namespace math

#endif // !MATH_VEC3_H
