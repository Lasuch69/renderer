#include <cassert>
#include <cstring>

#include "mat4.h"
#include "vec4.h"

using namespace math;

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

	mat4 out;
	memcpy(&out, data, sizeof(mat4));
	return out;
}

mat4 mat4::operator*(const mat4 &m) const {
	return _mul(&m, this);
}

void mat4::operator*=(const mat4 &m) {
	*this = _mul(&m, this);
}

mat4::mat4(float _v) {
	x.x = _v;
	y.y = _v;
	z.z = _v;
	w.w = _v;
}

mat4::mat4(const vec4 &_x, const vec4 &_y, const vec4 &_z, const vec4 &_w) {
	x = _x;
	y = _y;
	z = _z;
	w = _w;
}
