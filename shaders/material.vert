#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec2 inTexCoord;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec3 outTangent;
layout(location = 2) out vec3 outBitangent;
layout(location = 3) out vec3 outNormal;
layout(location = 4) out vec2 outTexCoord;

layout(push_constant) uniform MaterialConstants {
	mat4 PROJECTION_VIEW_MATRIX;
	mat4 MODEL_MATRIX;
};

void main() {
	vec3 tangent = normalize(vec3(MODEL_MATRIX * vec4(inTangent, 0.0)));
	vec3 normal = normalize(vec3(MODEL_MATRIX * vec4(inNormal, 0.0)));

	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(normal, tangent);

	vec4 position4 = MODEL_MATRIX * vec4(inPosition, 1.0);
	vec3 position = vec3(position4) / position4.w;
	
	outPosition = position;
	outTangent = tangent;
	outBitangent = bitangent;
	outNormal = normal;
	outTexCoord = inTexCoord;

	gl_Position = PROJECTION_VIEW_MATRIX * MODEL_MATRIX * vec4(position, 1.0);
}
