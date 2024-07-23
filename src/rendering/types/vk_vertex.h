#ifndef VK_VERTEX_H
#define VK_VERTEX_H

#include <cstddef>
#include <cstdint>

#include <vulkan/vulkan_core.h>

#include "common/vertex.h"

static VkVertexInputBindingDescription vertexBindings() {
	return {
		.binding = 0,
		.stride = sizeof(Vertex),
		.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
	};
}

static VkVertexInputAttributeDescription *vertexAttributes(uint32_t *count) {
	static VkVertexInputAttributeDescription attributes[] = {
		VkVertexInputAttributeDescription{
				.location = 0,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, position),
		},
		VkVertexInputAttributeDescription{
				.location = 1,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, normal),
		},
		VkVertexInputAttributeDescription{
				.location = 2,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, tangent),
		},
		VkVertexInputAttributeDescription{
				.location = 3,
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vertex, texCoord),
		},
	};

	*count = sizeof(attributes) / sizeof(attributes[0]);
	return attributes;
}

#endif // !VK_VERTEX_H
