vec2 sign_not_zero(vec2 v) {
	return vec2((v.x >= 0.0) ? 1.0 : -1.0, (v.y >= 0.0) ? 1.0 : -1.0);
}

vec3 oct_to_float32x3(vec2 e) {
	vec3 v = vec3(e.xy, 1.0 - abs(e.x) - abs(e.y));
	if (v.z < 0) v.xy = (1.0 - abs(v.yx)) * sign_not_zero(v.xy);
	return normalize(v);
}

uvec2 uint8_to_uint4x2(uint v) {
	return uvec2(v & 0xf, v >> 4);
}

vec2 unorm8x3_to_unorm12x2(vec3 v) {
	uvec3 u = uvec3(v * 255.0);
	uint x = ((u.y & 0xf) << 8) + u.x;
	uint y = (u.z << 4) + (u.y >> 4);
	return vec2(x, y) * vec2(1.0 / 4095.0);
}
