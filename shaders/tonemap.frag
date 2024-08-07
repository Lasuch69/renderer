#version 450

#extension GL_GOOGLE_include_directive : enable

#include "include/tonemap_incl.glsl"

layout(location = 0) out vec4 out_frag_color;

layout(binding = 0, input_attachment_index = 0) uniform subpassInput in_color;

const float EXPOSURE = 1.0;
const float WHITE = 8.0;
const float BLACK = 0.00017578;

void main() {
	vec3 color = subpassLoad(in_color).rgb * EXPOSURE;
	color = agx(color, WHITE, BLACK);
	color = agxEotf(color);

	out_frag_color = vec4(color, 1.0);
}
