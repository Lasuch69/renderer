#version 450

#extension GL_GOOGLE_include_directive : enable

#include "include/std_incl.glsl"

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_tangent;
layout(location = 2) in vec3 in_normal;
layout(location = 3) in vec3 in_tex_coord;
layout(location = 4) in uint in_tex_offset;

layout(location = 0) out vec3 out_position;
layout(location = 1) out vec3 out_tangent;
layout(location = 2) out vec3 out_bitangent;
layout(location = 3) out vec3 out_normal;
layout(location = 4) out vec2 out_tex_coord;

layout(push_constant) uniform ObjectConstants {
	mat4 PROJECTION_VIEW_MATRIX;
	mat4 MODEL_MATRIX;
	vec4 SIZE;
	vec4 OFFSET;
};

void main() {
	vec3 tangent = oct_to_float32x3(unorm8x3_to_unorm12x2(in_tangent) * 2.0 - 1.0);
	vec3 normal = oct_to_float32x3(unorm8x3_to_unorm12x2(in_normal) * 2.0 - 1.0);
	vec2 tex_coord = unorm8x3_to_unorm12x2(in_tex_coord) + vec2(uint8_to_uint4x2(in_tex_offset));

	const mat3 MODEL_NORMAL_MATRIX = mat3(MODEL_MATRIX);
	tangent = normalize(MODEL_NORMAL_MATRIX * tangent);
	normal = normalize(MODEL_NORMAL_MATRIX * normal);

	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(normal, tangent);

	vec3 position = in_position * SIZE.xyz + OFFSET.xyz;
	vec4 position_vec4 = MODEL_MATRIX * vec4(position, 1.0);

	float denom = 1.0 / position_vec4.w;
	position = position_vec4.xyz * denom;
	
	out_position = position;
	out_tangent = tangent;
	out_bitangent = bitangent;
	out_normal = normal;
	out_tex_coord = tex_coord;

	gl_Position = PROJECTION_VIEW_MATRIX * vec4(position, 1.0);
}
