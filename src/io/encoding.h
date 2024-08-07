#ifndef ENCODING_H
#define ENCODING_H

#include <cstdint>

#define UINT12_MAX 0xfff

const float ONE_DIV_UINT12_MAX = 1.0f / UINT12_MAX;

namespace encoding {

typedef struct {
	float x, y;
} float32x2;

// y:  0 - 11 bit
// x: 12 - 23 bit
typedef struct unorm12x2 {
	uint8_t data[3];
} unorm12x2;

inline float abs(float x) {
	uint32_t n = (*(uint32_t *)&x) & 0x7fffffff;
	return *(float *)&n;
}

inline float sign_not_zero(float v) {
	return (v >= 0.0) ? 1.0 : -1.0;
}

// Assumes input is normalized
inline float32x2 float32x3_to_oct(float x, float y, float z) {
	float denom = abs(x) + abs(y) + abs(z);
	denom = 1.0 / denom;

	float px = x * denom;
	float py = y * denom;

	float _x = z <= 0.0 ? (1.0 - abs(py)) * sign_not_zero(px) : px;
	float _y = z <= 0.0 ? (1.0 - abs(px)) * sign_not_zero(py) : py;

	return { _x, _y };
}

// Assumes input range of [0..1]
inline unorm12x2 float32x2_to_unorm12x2(const float32x2 &value) {
	uint16_t x = value.x * UINT12_MAX;
	uint16_t y = value.y * UINT12_MAX;

	uint32_t i = (y << 20) + ((x & 0xfff) << 8);
	uint8_t *data = (uint8_t *)&i;

	unorm12x2 out;
	out.data[0] = data[1];
	out.data[1] = data[2];
	out.data[2] = data[3];
	return out;
}

inline float32x2 unorm12x2_to_float32x2(const unorm12x2 &value) {
	uint32_t i = 0;
	uint8_t *data = (uint8_t *)&i;

	data[1] = value.data[0];
	data[2] = value.data[1];
	data[3] = value.data[2];

	uint16_t x = (i >> 8) & 0xfff;
	uint16_t y = (i >> 20) & 0xfff;

	float32x2 out;
	out.x = x * ONE_DIV_UINT12_MAX;
	out.y = y * ONE_DIV_UINT12_MAX;
	return out;
}

} // namespace encoding

#endif // !ENCODING_H
