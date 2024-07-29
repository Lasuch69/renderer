#ifndef MATH_VEC3_H
#define MATH_VEC3_H

namespace math {

class vec3;

vec3 cross(const vec3 &a, const vec3 &b);
float distance(const vec3 &a, const vec3 &b);
float dot(const vec3 &a, const vec3 &b);
float length(const vec3 &v);
vec3 normalize(const vec3 &v);

class vec3 {
public:
	float x, y, z;

	vec3 operator-() const;

	vec3 operator*(float v) const;
	vec3 operator/(float v) const;

	vec3 operator+(const vec3 &v) const;
	vec3 operator-(const vec3 &v) const;
	vec3 operator*(const vec3 &v) const;
	vec3 operator/(const vec3 &v) const;

	void operator*=(float v);
	void operator/=(float v);

	void operator+=(const vec3 &v);
	void operator-=(const vec3 &v);
	void operator*=(const vec3 &v);
	void operator/=(const vec3 &v);

	vec3() {}
	vec3(float _v);
	vec3(float _x, float _y, float _z);
};

} // namespace math

#endif // !MATH_VEC3_H
