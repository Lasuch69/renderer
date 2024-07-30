#version 450

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec3 vertTangent;
layout(location = 3) in vec2 vertTexCoord;

layout(location = 4) in vec4 MODEL_MATRIX_0;
layout(location = 5) in vec4 MODEL_MATRIX_1;
layout(location = 6) in vec4 MODEL_MATRIX_2;
layout(location = 7) in vec4 MODEL_MATRIX_3;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragTangent;
layout(location = 2) out vec3 fragBitangent;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec2 fragTexCoord;

// layout(location = 5) out uint instanceIndex;

layout(push_constant) uniform CameraConstants {
	mat4 PROJECTION_VIEW_MATRIX;
};

void main() {
	const mat4 MODEL_MATRIX = mat4(
		MODEL_MATRIX_0,
		MODEL_MATRIX_1,
		MODEL_MATRIX_2,
		MODEL_MATRIX_3
	);
	
	vec3 tangent = normalize(vec3(MODEL_MATRIX * vec4(vertTangent, 0.0)));
	vec3 normal = normalize(vec3(MODEL_MATRIX * vec4(vertNormal, 0.0)));

	tangent = normalize(tangent - dot(tangent, normal) * normal);
	vec3 bitangent = cross(normal, tangent);
	
	fragTangent = tangent;
	fragBitangent = bitangent;
	fragNormal = normal;
	
	vec4 vertPosition4 = MODEL_MATRIX * vec4(vertPosition, 1.0);
	fragPosition = vec3(vertPosition4) / vertPosition4.w;
	fragTexCoord = vertTexCoord;

	gl_Position = PROJECTION_VIEW_MATRIX * MODEL_MATRIX * vec4(vertPosition, 1.0);
	// instanceIndex = gl_InstanceIndex;
}
