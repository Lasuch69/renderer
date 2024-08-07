#version 450

layout(location = 0) in vec3 in_position;

layout(push_constant) uniform ObjectConstants {
	mat4 PROJECTION_VIEW_MATRIX;
	mat4 MODEL_MATRIX;
};

void main() {
	gl_Position = PROJECTION_VIEW_MATRIX * MODEL_MATRIX * vec4(in_position, 1.0);
}
