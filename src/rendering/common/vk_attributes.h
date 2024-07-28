#ifndef VK_ATTRIBUTES_H
#define VK_ATTRIBUTES_H

#include <vulkan/vulkan_core.h>

#include "common/vertex.h"

const VkVertexInputBindingDescription VERTEX_BINDING = {
	.binding = 0,
	.stride = sizeof(Vertex),
	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};

const VkVertexInputAttributeDescription VERTEX_ATTRIBUTES[] = {
	{
			.location = 0,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, position),
	},
	{
			.location = 1,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, normal),
	},
	{
			.location = 2,
			.binding = 0,
			.format = VK_FORMAT_R32G32B32_SFLOAT,
			.offset = offsetof(Vertex, tangent),
	},
	{
			.location = 3,
			.binding = 0,
			.format = VK_FORMAT_R32G32_SFLOAT,
			.offset = offsetof(Vertex, texCoord),
	},
};

typedef struct {
	float MODEL_MATRIX_0[4];
	float MODEL_MATRIX_1[4];
	float MODEL_MATRIX_2[4];
	float MODEL_MATRIX_3[4];
} Instance;

const VkVertexInputBindingDescription INSTANCE_BINDING = {
	.binding = 1,
	.stride = sizeof(Instance),
	.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE,
};

const VkVertexInputAttributeDescription INSTANCE_ATTRIBUTES[] = {
	{
			.location = 4,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(Instance, MODEL_MATRIX_0),
	},
	{
			.location = 5,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(Instance, MODEL_MATRIX_1),
	},
	{
			.location = 6,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(Instance, MODEL_MATRIX_2),
	},
	{
			.location = 7,
			.binding = 1,
			.format = VK_FORMAT_R32G32B32A32_SFLOAT,
			.offset = offsetof(Instance, MODEL_MATRIX_3),
	},
};

#endif // !VK_ATTRIBUTES_H
