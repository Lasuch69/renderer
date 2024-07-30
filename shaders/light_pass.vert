#version 450

const vec2 VERTEX_POSITION[3] = vec2[](
	vec2(-1.0, -1.0),
	vec2(-1.0, 3.0),
	vec2(3.0, -1.0)
);

void main() {
	gl_Position = vec4(VERTEX_POSITION[gl_VertexIndex], 0.0, 1.0);
}
