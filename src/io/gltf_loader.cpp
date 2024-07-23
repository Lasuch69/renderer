#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cgltf/cgltf.h>
#include <stb/stb_image.h>

#include <io/types/mesh.h>
#include <io/types/vertex.h>

#include <math/types/mat4.h>
#include <math/types/vec3.h>

#include "gltf_loader.h"

bool GLTFLoader::_checkAttributes(const cgltf_attribute *attributes, uint32_t attributeCount) {
	const char *REQUIRED_ATTRIBUTES[] = {
		"POSITION",
		"NORMAL",
		"TEXCOORD_0",
	};

	for (const char *requiredAttribute : REQUIRED_ATTRIBUTES) {
		bool isRequiredAttributeFound = false;
		for (uint32_t i = 0; i < attributeCount; i++) {
			if (strcmp(requiredAttribute, attributes[i].name) != 0)
				continue;

			isRequiredAttributeFound = true;
			break;
		}

		if (isRequiredAttributeFound)
			continue;

		return false;
	}

	return true;
}

math::mat4 GLTFLoader::_extractTransform(const cgltf_node &node) {
	if (node.has_matrix) {
		math::mat4 matrix;
		memcpy(&matrix, node.matrix, sizeof(float) * 16);
		return matrix;
	}

	return math::mat4(1.0f);
}

void GLTFLoader::_tangentsGenerate(const IndexArray &indices, VertexArray &vertices) {
	assert(indices.count % 3 == 0);

	math::vec3 *tangents = (math::vec3 *)calloc(vertices.count, sizeof(math::vec3));
	float *averages = (float *)calloc(vertices.count, sizeof(float));

	for (size_t i = 0; i < indices.count; i += 3) {
		const Vertex &v0 = vertices.data[indices.data[i + 0]];
		const Vertex &v1 = vertices.data[indices.data[i + 1]];
		const Vertex &v2 = vertices.data[indices.data[i + 2]];

		math::vec3 position0 = {
			v1.position[0] - v0.position[0],
			v1.position[1] - v0.position[1],
			v1.position[2] - v0.position[2],
		};

		math::vec3 position1 = {
			v2.position[0] - v0.position[0],
			v2.position[1] - v0.position[1],
			v2.position[2] - v0.position[2],
		};

		float texCoord0X = v1.texCoord[0] - v0.texCoord[0];
		float texCoord0Y = v1.texCoord[1] - v0.texCoord[1];

		float texCoord1X = v2.texCoord[0] - v0.texCoord[0];
		float texCoord1Y = v2.texCoord[1] - v0.texCoord[1];

		float r = 1.0 / (texCoord0X * texCoord1Y - texCoord0Y * texCoord1X);
		math::vec3 tangent = ((position0 * texCoord1Y) - (position1 * texCoord0Y)) * r;

		tangents[indices.data[i + 0]] += tangent;
		tangents[indices.data[i + 1]] += tangent;
		tangents[indices.data[i + 2]] += tangent;

		averages[indices.data[i + 0]] += 1.0;
		averages[indices.data[i + 1]] += 1.0;
		averages[indices.data[i + 2]] += 1.0;
	}

	for (uint32_t i = 0; i < vertices.count; i++) {
		float denom = 1.0 / averages[i];
		math::vec3 tangent = tangents[i] * denom;
		memcpy(vertices.data[i].tangent, &tangent, sizeof(tangent));
	}
}

Mesh GLTFLoader::_meshLoad(const cgltf_mesh &mesh) {
	Mesh _mesh = {};
	_mesh.primitiveCount = mesh.primitives_count;
	_mesh.primitives = (Primitive *)calloc(mesh.primitives_count, sizeof(Primitive));

	for (uint64_t i = 0; i < mesh.primitives_count; i++) {
		const cgltf_primitive &primitive = mesh.primitives[i];

		if (!_checkAttributes(primitive.attributes, primitive.attributes_count)) {
			fprintf(stderr, "Mesh: %s, primitive: %ld is missing required attributes!\n", mesh.name, i);
			continue;
		}

		IndexArray indices = {};
		indices.data = new uint32_t[primitive.indices->count];
		indices.count = primitive.indices->count;

		uint64_t offset = primitive.indices->buffer_view->offset;
		uint8_t *buffer = reinterpret_cast<uint8_t *>(primitive.indices->buffer_view->buffer->data);

		switch (primitive.indices->component_type) {
			case cgltf_component_type_r_16u:
				for (uint32_t idx = 0; idx < indices.count; idx++) {
					uint16_t element;
					memcpy(&element, &buffer[offset + (idx * sizeof(uint16_t))], sizeof(uint16_t));
					indices.data[idx] = element;
				}
				break;
			case cgltf_component_type_r_32u:
				memcpy(indices.data, &buffer[offset], indices.count * sizeof(uint32_t));
				break;
			default:
				break;
		}

		VertexArray vertices = {};
		vertices.data = nullptr;
		vertices.count = 0;

		AABB aabb = {};

		for (uint64_t attributeIndex = 0; attributeIndex < primitive.attributes_count; attributeIndex++) {
			if (strcmp("POSITION", primitive.attributes[attributeIndex].name) != 0)
				continue;

			const cgltf_accessor *positionAccessor = primitive.attributes[attributeIndex].data;

			vertices.data = new Vertex[positionAccessor->count];
			vertices.count = positionAccessor->count;

			// INFO: required in specification, probably check if valid anyway.
			// Always float (same as POSITION attribute).
			const float *min = positionAccessor->min;
			const float *max = positionAccessor->max;

			aabb.x = min[0];
			aabb.y = min[1];
			aabb.z = min[2];

			aabb.w = max[0] - min[0];
			aabb.h = max[1] - min[1];
			aabb.d = max[2] - min[2];
		}

		for (uint64_t attributeIndex = 0; attributeIndex < primitive.attributes_count; attributeIndex++) {
			const cgltf_attribute *attribute = &primitive.attributes[attributeIndex];

			if (attribute == nullptr)
				continue;

			const cgltf_accessor *accessor = attribute->data;
			const cgltf_buffer_view *bufferView = accessor->buffer_view;

			if (bufferView->buffer->data == nullptr)
				continue;

			const uint64_t begin = bufferView->offset;
			const uint64_t size = bufferView->size;

			const uint8_t *buffer = reinterpret_cast<uint8_t *>(bufferView->buffer->data);

			uint32_t vertexIdx = 0;
			switch (primitive.attributes[attributeIndex].type) {
				case cgltf_attribute_type_position:
					for (uint64_t offset = begin; offset < begin + size; offset += sizeof(float) * 3) {
						memcpy(vertices.data[vertexIdx].position, &buffer[offset], sizeof(float) * 3);
						vertexIdx++;
					}

					break;
				case cgltf_attribute_type_normal:
					for (uint64_t offset = begin; offset < begin + size; offset += sizeof(float) * 3) {
						memcpy(vertices.data[vertexIdx].normal, &buffer[offset], sizeof(float) * 3);
						vertexIdx++;
					}

					break;
				case cgltf_attribute_type_texcoord:
					// TODO: Handle other component types from specification
					for (uint64_t offset = begin; offset < begin + size; offset += sizeof(float) * 2) {
						memcpy(vertices.data[vertexIdx].texCoord, &buffer[offset], sizeof(float) * 2);
						vertexIdx++;
					}

					break;
				default:
					continue;
			}
		}

		_tangentsGenerate(indices, vertices);

		_mesh.primitives[i].aabb = aabb;
		_mesh.primitives[i].indices = indices;
		_mesh.primitives[i].vertices = vertices;
	}

	return _mesh;
}

void GLTFLoader::loadFile(const char *path) {
	cgltf_options options = {};
	cgltf_data *data = NULL;

	if (cgltf_parse_file(&options, path, &data) != cgltf_result_success)
		return;

	if (cgltf_load_buffers(&options, data, path) != cgltf_result_success)
		return;

	for (uint64_t i = 0; i < data->meshes_count; i++)
		_meshLoad(data->meshes[i]);

	cgltf_free(data);
}
