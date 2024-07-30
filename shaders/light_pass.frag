#version 450

// gbuffer
layout(binding = 0, input_attachment_index = 0) uniform subpassInput fragAlbedo;
layout(binding = 1, input_attachment_index = 1) uniform subpassInput fragNormal;
layout(binding = 2, input_attachment_index = 2) uniform subpassInput fragRoughnessMetallic;
layout(binding = 3, input_attachment_index = 3) uniform subpassInput fragDepth;

layout(location = 0) out vec4 outFragColor;

void main() {
	outFragColor = subpassLoad(fragNormal);
}
