#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cgltf/cgltf.h>
#include <stb/stb_image.h>

#include <common/mesh.h>
#include <common/scene.h>
#include <common/vertex.h>

#include <math/types/mat4.h>
#include <math/types/vec3.h>

#include "loader.h"

math::mat4 _transformExtract(const cgltf_node &node) {
	if (node.has_matrix) {
		math::mat4 matrix;
		memcpy(&matrix, node.matrix, sizeof(float) * 16);
		return matrix;
	}

	return math::mat4(1.0f);
}

bool _checkRequiredAttributes(const cgltf_primitive &primitive) {
	const char *REQUIRED_ATTRIBUTES[] = {
		"POSITION",
		"NORMAL",
		"TEXCOORD_0",
	};

	for (const char *requiredAttribute : REQUIRED_ATTRIBUTES) {
		bool isRequiredAttributeFound = false;
		for (size_t i = 0; i < primitive.attributes_count; i++) {
			if (strcmp(requiredAttribute, primitive.attributes[i].name) != 0)
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

void _primitiveBounds(const cgltf_primitive &primitive, math::vec3 *size, math::vec3 *offset) {
	for (size_t attributeIndex = 0; attributeIndex < primitive.attributes_count; attributeIndex++) {
		if (strcmp("POSITION", primitive.attributes[attributeIndex].name) != 0)
			continue;

		const cgltf_accessor *accessor = primitive.attributes[attributeIndex].data;

		// INFO: required in specification, vector 3 (float);
		const float *min = accessor->min;
		const float *max = accessor->max;

		*size = { max[0] - min[0], max[1] - min[1], max[2] - min[2] };
		*offset = { min[0], min[1], min[2] };
		return;
	}

	*size = math::vec3(0.0f);
	*offset = math::vec3(0.0f);

	printf("Could not find POSITION attribute!");
	printf("Getting primitive bounding box failed!");
}

IndexArray _primitiveIndices(const cgltf_primitive &primitive) {
	const cgltf_buffer_view *bufferView = primitive.indices->buffer_view;

	const size_t begin = bufferView->offset;
	const uint8_t *buffer = reinterpret_cast<uint8_t *>(bufferView->buffer->data);
	const size_t componentSize = cgltf_component_size(primitive.indices->component_type);

	uint8_t *data = (uint8_t *)calloc(primitive.indices->count, sizeof(uint32_t));

	for (size_t i = 0; i < primitive.indices->count; i++) {
		memcpy(&data[i * sizeof(uint32_t)], &buffer[begin + (i * componentSize)], componentSize);
	}

	return { reinterpret_cast<uint32_t *>(data), (uint32_t)primitive.indices->count };
}

VertexArray _primitiveVertices(const cgltf_primitive &primitive) {
	Vertex *vertices = nullptr;
	uint32_t vertexCount = 0;

	for (size_t i = 0; i < primitive.attributes_count; i++) {
		if (strcmp("POSITION", primitive.attributes[i].name) != 0)
			continue;

		const cgltf_accessor *accessor = primitive.attributes[i].data;

		vertices = new Vertex[accessor->count];
		vertexCount = accessor->count;
	}

	for (size_t i = 0; i < primitive.attributes_count; i++) {
		const cgltf_attribute &attribute = primitive.attributes[i];

		const cgltf_accessor *accessor = attribute.data;
		const cgltf_buffer_view *bufferView = accessor->buffer_view;

		if (bufferView->buffer->data == nullptr)
			continue;

		const size_t begin = bufferView->offset;
		const size_t size = bufferView->size;

		const uint8_t *buffer = reinterpret_cast<uint8_t *>(bufferView->buffer->data);

		uint32_t idx = 0;
		switch (attribute.type) {
			case cgltf_attribute_type_position:
				for (size_t offset = begin; offset < begin + size; offset += sizeof(float) * 3) {
					memcpy(vertices[idx].position, &buffer[offset], sizeof(float) * 3);
					idx++;
				}

				break;
			case cgltf_attribute_type_normal:
				for (size_t offset = begin; offset < begin + size; offset += sizeof(float) * 3) {
					memcpy(vertices[idx].normal, &buffer[offset], sizeof(float) * 3);
					idx++;
				}

				break;
			case cgltf_attribute_type_texcoord:
				// TODO: Handle other component types from specification
				for (size_t offset = begin; offset < begin + size; offset += sizeof(float) * 2) {
					memcpy(vertices[idx].texCoord, &buffer[offset], sizeof(float) * 2);
					idx++;
				}

				break;
			default:
				continue;
		}
	}

	VertexArray vertexArray = {};
	vertexArray.data = (uint8_t *)vertices;
	vertexArray.count = vertexCount;
	vertexArray.isPacked = false;

	return vertexArray;
}

bool _tangentsCalculate(const IndexArray &indexArray, VertexArray &vertexArray) {
	if (indexArray.count % 3 != 0)
		return false;

	math::vec3 *tangents = (math::vec3 *)calloc(vertexArray.count, sizeof(math::vec3));
	float *averages = (float *)calloc(vertexArray.count, sizeof(float));

	const uint32_t *indices = indexArray.data;
	Vertex *vertices = (Vertex *)vertexArray.data;

	for (uint32_t i = 0; i < indexArray.count; i += 3) {
		const Vertex &v0 = vertices[indices[i + 0]];
		const Vertex &v1 = vertices[indices[i + 1]];
		const Vertex &v2 = vertices[indices[i + 2]];

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

		tangents[indexArray.data[i + 0]] += tangent;
		tangents[indexArray.data[i + 1]] += tangent;
		tangents[indexArray.data[i + 2]] += tangent;

		averages[indexArray.data[i + 0]] += 1.0;
		averages[indexArray.data[i + 1]] += 1.0;
		averages[indexArray.data[i + 2]] += 1.0;
	}

	for (uint32_t i = 0; i < vertexArray.count; i++) {
		float denom = 1.0 / averages[i];
		vertices[i].tangent[0] = tangents[i].x * denom;
		vertices[i].tangent[1] = tangents[i].y * denom;
		vertices[i].tangent[2] = tangents[i].z * denom;
	}

	return true;
}

Scene *Loader::sceneLoadGlTF(const char *path) {
	cgltf_options options = {};
	cgltf_data *data = NULL;

	if (cgltf_parse_file(&options, path, &data) != cgltf_result_success)
		return nullptr;

	if (cgltf_load_buffers(&options, data, path) != cgltf_result_success)
		return nullptr;

	if (cgltf_validate(data) != cgltf_result_success)
		return nullptr;

	Node *nodes = new Node[data->nodes_count];
	Mesh *meshes = new Mesh[data->meshes_count];

	for (size_t nodeIdx = 0; nodeIdx < data->nodes_count; nodeIdx++) {
		const cgltf_node &_node = data->nodes[nodeIdx];

		if (_node.mesh != nullptr) {
			nodes[nodeIdx].hasMesh = true;
			nodes[nodeIdx].meshIndex = cgltf_mesh_index(data, _node.mesh);
		} else {
			nodes[nodeIdx].hasMesh = false;
			nodes[nodeIdx].meshIndex = 0;
		}

		math::mat4 transform = _transformExtract(_node);
		memcpy(nodes[nodeIdx].transform, &transform, sizeof(float) * 16);
	}

	for (size_t meshIdx = 0; meshIdx < data->meshes_count; meshIdx++) {
		const cgltf_mesh &_mesh = data->meshes[meshIdx];

		meshes[meshIdx].primitives = new Primitive[_mesh.primitives_count];
		meshes[meshIdx].primitiveCount = _mesh.primitives_count;

		for (size_t primitiveIdx = 0; primitiveIdx < _mesh.primitives_count; primitiveIdx++) {
			const cgltf_primitive &_primitive = _mesh.primitives[primitiveIdx];

			if (!_checkRequiredAttributes(_primitive)) {
				fprintf(stderr, "Mesh: %s, primitive id: %ld is missing required attributes!\n", _mesh.name,
						primitiveIdx);

				meshes[meshIdx].primitives[primitiveIdx] = {};
				continue;
			}

			math::vec3 size;
			math::vec3 offset;
			_primitiveBounds(_primitive, &size, &offset);

			IndexArray indices = _primitiveIndices(_primitive);
			VertexArray vertices = _primitiveVertices(_primitive);

			if (!_tangentsCalculate(indices, vertices)) {
				fprintf(stderr, "Mesh: %s, primitive id: %ld failed to calculate tangents!\n", _mesh.name,
						primitiveIdx);

				free(indices.data);
				free(vertices.data);

				meshes[meshIdx].primitives[primitiveIdx] = {};
				continue;
			}

			Primitive primitive = {};
			primitive.indices = indices;
			primitive.vertices = vertices;
			primitive.materialIndex = cgltf_material_index(data, _primitive.material);

			memcpy(primitive.size, &size, sizeof(size));
			memcpy(primitive.offset, &offset, sizeof(offset));

			meshes[meshIdx].primitives[primitiveIdx] = primitive;
		}
	}

	Scene *scene = new Scene;
	scene->nodes = nodes;
	scene->nodeCount = data->nodes_count;
	scene->meshes = meshes;
	scene->meshCount = data->meshes_count;

	cgltf_free(data);
	return scene;
}
