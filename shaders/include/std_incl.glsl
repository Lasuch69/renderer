const float ONE_DIV_16 = 0.0625;

vec2 uint8_to_uint4x2(float n) {
	float y = floor(n * ONE_DIV_16);
	float x = n - (y * 16.0);
	return vec2(x, y);
}

vec2 unorm8x3_to_snorm12x2(vec3 u) {
	u *= 255.0;
	u.y *= ONE_DIV_16;
	vec2 s = vec2(u.x * 16.0 + floor(u.y), fract(u.y) * (16.0 * 256.0) + u.z);
	return clamp(s * (1.0 / 2047.0) - 1.0, vec2(-1.0), vec2(1.0));
}

vec2 signNotZero(vec2 v) {
	return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? 1.0 : -1.0);
}

vec3 oct_to_float32x3(vec2 e) {
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * signNotZero(v.xy);
	return normalize(v);
}
