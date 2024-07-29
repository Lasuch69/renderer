#ifndef MATH_MAT4_H
#define MATH_MAT4_H

#include "vec4.h"

namespace math {

class mat4 {
public:
	vec4 x, y, z, w;

	mat4 operator*(const mat4 &m) const;
	void operator*=(const mat4 &m);

	mat4() {}
	mat4(float _v);
	mat4(const vec4 &_x, const vec4 &_y, const vec4 &_z, const vec4 &_w);
};

} // namespace math

#endif // !MATH_MAT4_H
