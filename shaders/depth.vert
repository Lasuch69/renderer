#version 450

layout(location = 0) in vec3 inPosition;

layout(push_constant) uniform ObjectConstants {
	mat4 PROJECTION_VIEW_MATRIX;
	mat4 MODEL_MATRIX;
};

void main() {
	vec4 position4 = MODEL_MATRIX * vec4(inPosition, 1.0);
	vec3 position = vec3(position4) / position4.w;
	
	gl_Position = PROJECTION_VIEW_MATRIX * MODEL_MATRIX * vec4(position, 1.0);
}
