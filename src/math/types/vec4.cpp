#include <cassert>
#include <cmath>

#include "vec3.h"
#include "vec4.h"

using namespace math;

float math::dot(const vec4 &a, const vec4 &b) {
	return (a.x * b.x) + (a.y * b.y) + (a.z * b.z) + (a.w * b.w);
}

float math::length(const vec4 &v) {
	return std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w));
}

vec4 math::normalize(const vec4 &v) {
	float denom = 1.0 / std::sqrt((v.x * v.x) + (v.y * v.y) + (v.z * v.z) + (v.w * v.w));
	return v * denom;
}

vec4 vec4::operator-() const {
	return {
		-x,
		-y,
		-z,
		-w,
	};
}

vec4 vec4::operator*(float v) const {
	return {
		x * v,
		y * v,
		z * v,
		w * v,
	};
}

vec4 vec4::operator/(float v) const {
	return {
		x / v,
		y / v,
		z / v,
		w / v,
	};
}

vec4 vec4::operator+(const vec4 &v) const {
	return {
		x + v.x,
		y + v.y,
		z + v.z,
		w + v.w,
	};
}

vec4 vec4::operator-(const vec4 &v) const {
	return {
		x - v.x,
		y - v.y,
		z - v.z,
		w - v.w,
	};
}

vec4 vec4::operator*(const vec4 &v) const {
	return {
		x * v.x,
		y * v.y,
		z * v.z,
		w * v.w,
	};
}

vec4 vec4::operator/(const vec4 &v) const {
	return {
		x / v.x,
		y / v.y,
		z / v.z,
		w / v.w,
	};
}

void vec4::operator*=(float v) {
	x *= v;
	y *= v;
	z *= v;
	w *= v;
}

void vec4::operator/=(float v) {
	x /= v;
	y /= v;
	z /= v;
	w /= v;
}

void vec4::operator+=(const vec4 &v) {
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
}

void vec4::operator-=(const vec4 &v) {
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
}

void vec4::operator*=(const vec4 &v) {
	x *= v.x;
	y *= v.y;
	z *= v.z;
	w *= v.w;
}

void vec4::operator/=(const vec4 &v) {
	x /= v.x;
	y /= v.y;
	z /= v.z;
	w /= v.w;
}

vec4::vec4(float _v) {
	x = _v;
	y = _v;
	z = _v;
	w = _v;
}

vec4::vec4(const vec3 &_xyz, float _w) {
	x = _xyz.x;
	y = _xyz.y;
	z = _xyz.z;
	w = _w;
}

vec4::vec4(float _x, float _y, float _z, float _w) {
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}
