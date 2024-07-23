#ifndef MATH_MAT3_H
#define MATH_MAT3_H

#include "vec3.h"

namespace math {

class mat3 {
public:
	vec3 x, y, z;

	mat3 operator*(const mat3 &m) const;
	void operator*=(const mat3 &m);

	mat3() {}
	mat3(float _v);
	mat3(const vec3 &_x, const vec3 &_y, const vec3 &_z);
};

} // namespace math

#endif // !MATH_MAT3_H
