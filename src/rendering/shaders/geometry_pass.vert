#version 450

layout(location = 0) in vec3 vertPosition;
layout(location = 1) in vec3 vertNormal;
layout(location = 2) in vec3 vertTangent;
layout(location = 3) in vec2 vertTexCoord;

layout(location = 4) in vec4 modelMatrix0;
layout(location = 5) in vec4 modelMatrix1;
layout(location = 6) in vec4 modelMatrix2;
layout(location = 7) in vec4 modelMatrix3;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec3 fragTangent;
layout(location = 2) out vec3 fragBitangent;
layout(location = 3) out vec3 fragNormal;
layout(location = 4) out vec2 fragTexCoord;

layout(location = 5) out uint instanceIndex;

layout(push_constant) uniform CameraConstant {
	mat4 projView;
};

void main() {
	mat4 model = mat4(modelMatrix0, modelMatrix1, modelMatrix2, modelMatrix3);
	vec3 T = normalize(vec3(model * vec4(vertTangent, 0.0)));
	vec3 N = normalize(vec3(model * vec4(vertNormal, 0.0)));

	T = normalize(T - dot(T, N) * N);
	vec3 B = cross(N, T);
	
	fragTangent = T;
	fragBitangent = B;
	fragNormal = N;
	
	vec4 vertPosition4 = model * vec4(vertPosition, 1.0);
	fragPosition = vec3(vertPosition4) / vertPosition4.w;
	fragTexCoord = vertTexCoord;

	gl_Position = projView * model * vec4(vertPosition, 1.0);
	instanceIndex = gl_InstanceIndex;
}
