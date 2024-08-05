#ifndef VK_VERTEX_H
#define VK_VERTEX_H

#include <cstddef>
#include <vulkan/vulkan_core.h>

#include <common/vertex.h>

const VkVertexInputBindingDescription VERTEX_BINDING = {
	.binding = 0,
	.stride = sizeof(PackedVertex),
	.inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
};

const VkVertexInputAttributeDescription VERTEX_ATTRIBUTES[] = {
	{ 0, 0, VK_FORMAT_R16G16B16_UNORM, offsetof(PackedVertex, position) },
	{ 1, 0, VK_FORMAT_R8G8B8_UNORM, offsetof(PackedVertex, tangent) },
	{ 2, 0, VK_FORMAT_R8G8B8_UNORM, offsetof(PackedVertex, normal) },
	{ 3, 0, VK_FORMAT_R8G8B8_UNORM, offsetof(PackedVertex, texCoord) },
	{ 4, 0, VK_FORMAT_R8_UINT, offsetof(PackedVertex, texOffset) },
};

#endif // !VK_VERTEX_H
