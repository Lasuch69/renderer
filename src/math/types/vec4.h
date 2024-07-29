#ifndef MATH_VEC4_H
#define MATH_VEC4_H

namespace math {

class vec3;
class vec4;

float dot(const vec4 &a, const vec4 &b);
float length(const vec4 &v);
vec4 normalize(const vec4 &v);

class vec4 {
public:
	float x, y, z, w;

	vec4 operator-() const;

	vec4 operator*(float v) const;
	vec4 operator/(float v) const;

	vec4 operator+(const vec4 &v) const;
	vec4 operator-(const vec4 &v) const;
	vec4 operator*(const vec4 &v) const;
	vec4 operator/(const vec4 &v) const;

	void operator*=(float v);
	void operator/=(float v);

	void operator+=(const vec4 &v);
	void operator-=(const vec4 &v);
	void operator*=(const vec4 &v);
	void operator/=(const vec4 &v);

	vec4() {}
	vec4(float _v);
	vec4(const vec3 &_xyz, float _w);
	vec4(float _x, float _y, float _z, float _w);
};

}; // namespace math

#endif // !MATH_VEC4_H
