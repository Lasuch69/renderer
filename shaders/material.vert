#version 450

#extension GL_GOOGLE_include_directive : enable

#include "include/std_incl.glsl"

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inTangent;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTexCoord;
layout(location = 4) in uint inTexOffset;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outTangent;
layout(location = 2) out vec3 outBitangent;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec2 outTexCoord;

layout(push_constant) uniform ObjectConstants {
	mat4 PROJECTION_VIEW_MATRIX;
	mat4 MODEL_MATRIX;
	vec4 SIZE;
	vec4 OFFSET;
};

void main() {
	vec3 tangent = oct_to_float32x3(unorm8x3_to_snorm12x2(inTangent));
	vec3 normal = oct_to_float32x3(unorm8x3_to_snorm12x2(inNormal));
	vec2 texCoord = unorm8x3_to_snorm12x2(inTexCoord) * vec2(0.5) + vec2(0.5);
	texCoord = texCoord + uint8_to_uint4x2(inTexOffset);

	const mat3 MODEL_NORMAL_MATRIX = mat3(MODEL_MATRIX);
	tangent = normalize(MODEL_NORMAL_MATRIX * tangent);
	normal = normalize(MODEL_NORMAL_MATRIX * normal);

	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(normal, tangent);

	vec3 position = inPosition * SIZE.xyz + OFFSET.xyz;
	vec4 position4 = MODEL_MATRIX * vec4(position, 1.0);

	float denom = 1.0 / position4.w;
	position = position4.xyz * denom;
	
	outPosition = position;
	outTangent = tangent;
	outBitangent = bitangent;
	outNormal = normal;
	outTexCoord = texCoord;

	gl_Position = PROJECTION_VIEW_MATRIX * vec4(position, 1.0);
}
