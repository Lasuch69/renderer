#ifndef VK_ATTRIBUTES_H
#define VK_ATTRIBUTES_H

#include <io/vertex.h>
#include <vulkan/vulkan_core.h>

const VkVertexInputBindingDescription VERTEX_BINDING = { 0, sizeof(Vertex), VK_VERTEX_INPUT_RATE_VERTEX };

const VkVertexInputAttributeDescription VERTEX_ATTRIBUTES[] = {
	{ 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position) },
	{ 1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal) },
	{ 2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, tangent) },
	{ 3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, texCoord) },
};

#endif // !VK_ATTRIBUTES_H
