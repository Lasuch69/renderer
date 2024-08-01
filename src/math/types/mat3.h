#ifndef MATH_MAT3_H
#define MATH_MAT3_H

#include "vec3.h"

namespace math {

struct mat3 {
private:
	static mat3 _mul(const mat3 *a, const mat3 *b) {
		vec3 rows[3] = { a->x, a->y, a->z };

		vec3 columns[3] = {
			{ b->x.x, b->y.x, b->z.x },
			{ b->x.y, b->y.y, b->z.y },
			{ b->x.z, b->y.z, b->z.z },
		};

		float data[3][3];
		for (int i = 0; i < 3; i++) {
			data[i][0] = dot(rows[i], columns[0]);
			data[i][1] = dot(rows[i], columns[1]);
			data[i][2] = dot(rows[i], columns[2]);
		}

		return {
			{ data[0][0], data[0][1], data[0][2] },
			{ data[1][0], data[1][1], data[1][2] },
			{ data[2][0], data[2][1], data[2][2] },
		};
	}

public:
	vec3 x, y, z;

	inline vec3 operator*(const vec3 &v) const {
		return { dot(v, x), dot(v, y), dot(v, z) };
	}

	inline mat3 operator*(const mat3 &m) const {
		return _mul(&m, this);
	}

	inline void operator*=(const mat3 &m) {
		*this = _mul(&m, this);
	}

	mat3() {}

	mat3(float _v) {
		x.x = _v;
		y.y = _v;
		z.z = _v;
	}

	mat3(const vec3 &_x, const vec3 &_y, const vec3 &_z) {
		x = _x;
		y = _y;
		z = _z;
	}
};

} // namespace math

#endif // !MATH_MAT3_H
