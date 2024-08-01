#ifndef MATH_MAT4_H
#define MATH_MAT4_H

#include "vec4.h"

namespace math {

struct mat4 {
private:
	static mat4 _mul(const mat4 *a, const mat4 *b) {
		vec4 rows[4] = { a->x, a->y, a->z, a->w };

		vec4 columns[4] = {
			{ b->x.x, b->y.x, b->z.x, b->w.x },
			{ b->x.y, b->y.y, b->z.y, b->w.y },
			{ b->x.z, b->y.z, b->z.z, b->w.z },
			{ b->x.w, b->y.w, b->z.w, b->w.w },
		};

		float data[4][4];
		for (int i = 0; i < 4; i++) {
			data[i][0] = dot(rows[i], columns[0]);
			data[i][1] = dot(rows[i], columns[1]);
			data[i][2] = dot(rows[i], columns[2]);
			data[i][3] = dot(rows[i], columns[3]);
		}

		return {
			{ data[0][0], data[0][1], data[0][2], data[0][3] },
			{ data[1][0], data[1][1], data[1][2], data[1][3] },
			{ data[2][0], data[2][1], data[2][2], data[2][3] },
			{ data[3][0], data[3][1], data[3][2], data[3][3] },
		};
	}

public:
	vec4 x, y, z, w;

	inline vec4 operator*(const vec4 &v) const {
		return { dot(v, x), dot(v, y), dot(v, z), dot(v, w) };
	}

	inline mat4 operator*(const mat4 &m) const {
		return _mul(&m, this);
	}

	inline void operator*=(const mat4 &m) {
		*this = _mul(&m, this);
	}

	mat4() {}

	mat4(float _v) {
		x.x = _v;
		y.y = _v;
		z.z = _v;
		w.w = _v;
	}

	mat4(const vec4 &_x, const vec4 &_y, const vec4 &_z, const vec4 &_w) {
		x = _x;
		y = _y;
		z = _z;
		w = _w;
	}
};

} // namespace math

#endif // !MATH_MAT4_H
