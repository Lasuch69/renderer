#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <cgltf/cgltf.h>
#include <stb/stb_image.h>

#include "common/mesh.h"
#include "common/vertex.h"

#include "math/types/vec3.h"

#include "gltf_loader.h"

bool checkRequiredAttributes(const cgltf_attribute *attributes, uint32_t attributeCount) {
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

void tangentsGenerate(const uint32_t *indices, uint32_t indexCount, Vertex *vertices, uint32_t vertexCount) {
	assert(indexCount % 3 == 0);

	float *averages = (float *)calloc(vertexCount, sizeof(float));
	math::vec3 *tangents = (math::vec3 *)calloc(vertexCount, sizeof(math::vec3));

	for (size_t i = 0; i < indexCount; i += 3) {
		const Vertex &v0 = vertices[indices[i + 0]];
		const Vertex &v1 = vertices[indices[i + 1]];
		const Vertex &v2 = vertices[indices[i + 2]];

		float position0[3];
		position0[0] = v1.position[0] - v0.position[0];
		position0[1] = v1.position[1] - v0.position[1];
		position0[2] = v1.position[2] - v0.position[2];

		float position1[3];
		position1[0] = v2.position[0] - v0.position[0];
		position1[1] = v2.position[1] - v0.position[1];
		position1[2] = v2.position[2] - v0.position[2];

		float texCoord0[2];
		texCoord0[0] = v1.texCoord[0] - v0.texCoord[0];
		texCoord0[1] = v1.texCoord[1] - v0.texCoord[1];

		float texCoord1[2];
		texCoord1[0] = v2.texCoord[0] - v0.texCoord[0];
		texCoord1[1] = v2.texCoord[1] - v0.texCoord[1];

		float r = 1.0 / (texCoord0[0] * texCoord1[1] - texCoord0[1] * texCoord1[0]);

		math::vec3 tangent;
		tangent.x = (position0[0] * texCoord1[1] - position1[0] * texCoord0[1]) * r;
		tangent.y = (position0[1] * texCoord1[1] - position1[1] * texCoord0[1]) * r;
		tangent.z = (position0[2] * texCoord1[1] - position1[2] * texCoord0[1]) * r;

		tangents[indices[i + 0]] += tangent;
		tangents[indices[i + 1]] += tangent;
		tangents[indices[i + 2]] += tangent;

		averages[indices[i + 0]] += 1.0;
		averages[indices[i + 1]] += 1.0;
		averages[indices[i + 2]] += 1.0;
	}

	for (uint32_t i = 0; i < vertexCount; i++) {
		float denom = 1.0 / averages[i];
		math::vec3 tangent = tangents[i] * denom;
		memcpy(vertices[i].tangent, &tangent, sizeof(tangent));
	}
}

Mesh meshLoad(const cgltf_mesh &mesh) {
	Mesh _mesh = {};
	_mesh.primitiveCount = mesh.primitives_count;
	_mesh.primitives = (Primitive *)calloc(mesh.primitives_count, sizeof(Primitive));

	for (uint64_t i = 0; i < mesh.primitives_count; i++) {
		const cgltf_primitive &primitive = mesh.primitives[i];

		if (!checkRequiredAttributes(primitive.attributes, primitive.attributes_count)) {
			fprintf(stderr, "Mesh: %s, primitive: %ld is missing required attributes!\n", mesh.name, i);
			continue;
		}

		uint32_t indexCount = primitive.indices->count;
		uint32_t *indices = new uint32_t[primitive.indices->count];

		uint64_t offset = primitive.indices->buffer_view->offset;
		uint8_t *buffer = reinterpret_cast<uint8_t *>(primitive.indices->buffer_view->buffer->data);

		switch (primitive.indices->component_type) {
			case cgltf_component_type_r_16u:
				for (uint32_t idx = 0; idx < indexCount; idx++) {
					uint16_t element;
					memcpy(&element, &buffer[offset + (idx * sizeof(uint16_t))], sizeof(uint16_t));
					indices[idx] = element;
				}
				break;
			case cgltf_component_type_r_32u:
				memcpy(indices, &buffer[offset], indexCount * sizeof(uint32_t));
				break;
			default:
				break;
		}

		uint32_t vertexCount = 0;
		Vertex *vertices = nullptr;

		// AABB aabb = {};

		for (uint64_t attributeIndex = 0; attributeIndex < primitive.attributes_count; attributeIndex++) {
			if (strcmp("POSITION", primitive.attributes[attributeIndex].name) != 0)
				continue;

			const cgltf_accessor *positionAccessor = primitive.attributes[attributeIndex].data;

			vertexCount = positionAccessor->count;
			vertices = new Vertex[positionAccessor->count];

			/*
			// INFO: min and max is required in specification, probably should implement checks anyway.
			// Component type should always be float (same as POSITION attribute).
			const float *min = positionAccessor->min;
			const float *max = positionAccessor->max;

			aabb.x = min[0];
			aabb.y = min[1];
			aabb.z = min[2];
			aabb.w = max[0] - min[0];
			aabb.h = max[1] - min[1];
			aabb.d = max[2] - min[2];
			*/
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
						memcpy(vertices[vertexIdx].position, &buffer[offset], sizeof(float) * 3);
						vertexIdx++;
					}

					break;
				case cgltf_attribute_type_normal:
					for (uint64_t offset = begin; offset < begin + size; offset += sizeof(float) * 3) {
						memcpy(vertices[vertexIdx].normal, &buffer[offset], sizeof(float) * 3);
						vertexIdx++;
					}

					break;
				case cgltf_attribute_type_texcoord:
					// TODO: Handle other component types from specification
					for (uint64_t offset = begin; offset < begin + size; offset += sizeof(float) * 2) {
						memcpy(vertices[vertexIdx].texCoord, &buffer[offset], sizeof(float) * 2);
						vertexIdx++;
					}

					break;
				default:
					continue;
			}
		}

		tangentsGenerate(indices, indexCount, vertices, vertexCount);

		for (uint32_t j = 0; j < indexCount; j++) {
			printf("%d, \n", indices[j]);
		}

		for (uint32_t j = 0; j < vertexCount; j++) {
			const Vertex &v = vertices[j];

			printf("{ { %.1f, %.1f, %.1f }, ", v.position[0], v.position[1], v.position[2]);
			printf(" { %.1f, %.1f, %.1f }, ", v.normal[0], v.normal[1], v.normal[2]);
			printf(" { %.1f, %.1f, %.1f }, ", v.tangent[0], v.tangent[1], v.tangent[2]);
			printf(" { %.3f, %.3f } }, \n", v.texCoord[0], v.texCoord[1]);
		}
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
		meshLoad(data->meshes[i]);

	cgltf_free(data);
}
