#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include <cgltf/cgltf.h>
#include <stb/stb_image.h>

#include <io/encoding.h>
#include <io/types/mesh.h>
#include <io/types/node.h>
#include <io/types/scene.h>
#include <io/types/vertex.h>

#include <math/types/mat4.h>
#include <math/types/vec3.h>

#include "loader.h"

#define UINT4_MAX 0xf

const float ONE_DIV_UINT8_MAX = 1.0f / UINT8_MAX;
const float ONE_DIV_UINT16_MAX = 1.0f / UINT16_MAX;

using namespace encoding;
using namespace math;

inline uint32_t min(uint32_t a, uint32_t b) {
	return a < b ? a : b;
}

bool Loader::_tangents_calculate(const uint32_t *indices, size_t index_count, Vertex *vertices, size_t vertex_count) {
	if (index_count % 3 != 0)
		return false;

	vec3 *tangents = (vec3 *)calloc(vertex_count, sizeof(vec3));
	float *averages = (float *)calloc(vertex_count, sizeof(float));

	for (uint32_t i = 0; i < index_count; i += 3) {
		const Vertex &vert0 = vertices[indices[i + 0]];
		const Vertex &vert1 = vertices[indices[i + 1]];
		const Vertex &vert2 = vertices[indices[i + 2]];

		vec3 pos0 = {
			vert1.position[0] - vert0.position[0],
			vert1.position[1] - vert0.position[1],
			vert1.position[2] - vert0.position[2],
		};

		vec3 pos1 = {
			vert2.position[0] - vert0.position[0],
			vert2.position[1] - vert0.position[1],
			vert2.position[2] - vert0.position[2],
		};

		float tex0_x = vert1.tex_coord[0] - vert0.tex_coord[0];
		float tex0_y = vert1.tex_coord[1] - vert0.tex_coord[1];

		float tex1_x = vert2.tex_coord[0] - vert0.tex_coord[0];
		float tex1_y = vert2.tex_coord[1] - vert0.tex_coord[1];

		float r = 1.0 / (tex0_x * tex1_y - tex0_y * tex1_x);
		vec3 tangent = ((pos0 * tex1_y) - (pos1 * tex0_y)) * r;

		tangents[indices[i + 0]] += tangent;
		tangents[indices[i + 1]] += tangent;
		tangents[indices[i + 2]] += tangent;

		averages[indices[i + 0]] += 1.0;
		averages[indices[i + 1]] += 1.0;
		averages[indices[i + 2]] += 1.0;
	}

	for (uint32_t i = 0; i < vertex_count; i++) {
		vec3 &tangent = tangents[i];

		float denom = 1.0 / averages[i];
		tangent *= denom;

		tangent = normalize(tangent);
		vertices[i].tangent[0] = tangent.x;
		vertices[i].tangent[1] = tangent.y;
		vertices[i].tangent[2] = tangent.z;
	}

	return true;
}

PackedVertex Loader::_vertex_compress(const Vertex &vertex, const float *inverse_size, const float *offset) {
	PackedVertex packed = {};

	// pack position
	packed.position[0] = ((vertex.position[0] - offset[0]) * inverse_size[0]) * UINT16_MAX;
	packed.position[1] = ((vertex.position[1] - offset[1]) * inverse_size[1]) * UINT16_MAX;
	packed.position[2] = ((vertex.position[2] - offset[2]) * inverse_size[2]) * UINT16_MAX;

	// pack tangents
	float32x2 tangent_oct = float32x3_to_oct(vertex.tangent[0], vertex.tangent[1], vertex.tangent[2]);
	tangent_oct.x = (tangent_oct.x * 0.5f) + 0.5f;
	tangent_oct.y = (tangent_oct.y * 0.5f) + 0.5f;

	unorm12x2 tangent_unorm12x2 = float32x2_to_unorm12x2(tangent_oct);
	packed.tangent[0] = tangent_unorm12x2.data[0];
	packed.tangent[1] = tangent_unorm12x2.data[1];
	packed.tangent[2] = tangent_unorm12x2.data[2];

	// pack normals
	float32x2 normal_oct = float32x3_to_oct(vertex.normal[0], vertex.normal[1], vertex.normal[2]);
	normal_oct.x = (normal_oct.x * 0.5f) + 0.5f;
	normal_oct.y = (normal_oct.y * 0.5f) + 0.5f;

	unorm12x2 normal_unorm12x2 = float32x2_to_unorm12x2(normal_oct);
	packed.normal[0] = normal_unorm12x2.data[0];
	packed.normal[1] = normal_unorm12x2.data[1];
	packed.normal[2] = normal_unorm12x2.data[2];

	// pack texCoord
	float32x2 tex_coord = {
		vertex.tex_coord[0] - int(vertex.tex_coord[0]),
		vertex.tex_coord[1] - int(vertex.tex_coord[1]),
	};

	unorm12x2 tex_coord_unorm12x2 = float32x2_to_unorm12x2(tex_coord);
	packed.tex_coord[0] = tex_coord_unorm12x2.data[0];
	packed.tex_coord[1] = tex_coord_unorm12x2.data[1];
	packed.tex_coord[2] = tex_coord_unorm12x2.data[2];

	// TODO: Handle negative offset
	uint8_t texOffsetX = min(int(vertex.tex_coord[0]), UINT4_MAX);
	uint8_t texOffsetY = min(int(vertex.tex_coord[1]), UINT4_MAX);

	// integer texture offset packed as uint4x2
	packed.tex_offset = (texOffsetY << 4) + texOffsetX;

	return packed;
}

Primitive Loader::_primitive_load(const cgltf_data *cgltf_data, const cgltf_primitive &cgltf_primitive) {
	size_t index_count = 0;
	uint32_t *indices = nullptr;

	size_t vertex_count = 0;
	Vertex *vertices = nullptr;

	float size[3] = { 0.0f, 0.0f, 0.0f };
	float offset[3] = { 0.0f, 0.0f, 0.0f };

	{
		const cgltf_buffer_view *index_buffer_view = cgltf_primitive.indices->buffer_view;
		const uint8_t *index_buffer = (uint8_t *)index_buffer_view->buffer->data;

		const size_t index_begin = index_buffer_view->offset;

		index_count = cgltf_primitive.indices->count;
		indices = (uint32_t *)malloc(cgltf_primitive.indices->count * sizeof(uint32_t));

		switch (cgltf_primitive.indices->component_type) {
			case cgltf_component_type_r_16u:
				for (size_t i = 0; i < index_count; i++) {
					uint16_t index;

					size_t offset = i * sizeof(uint16_t);
					memcpy(&index, &index_buffer[index_begin + offset], sizeof(uint16_t));

					indices[i] = index;
				}
				break;
			case cgltf_component_type_r_32u:
				memcpy(indices, &index_buffer[index_begin], index_count * sizeof(uint32_t));
				break;
			default:
				throw std::runtime_error("Index component type is invalid!");
		}
	}

	for (size_t attribute_index = 0; attribute_index < cgltf_primitive.attributes_count; attribute_index++) {
		if (cgltf_primitive.attributes[attribute_index].type != cgltf_attribute_type_position)
			continue;

		const cgltf_accessor *position_accessor = cgltf_primitive.attributes[attribute_index].data;

		vertex_count = position_accessor->count;
		vertices = (Vertex *)malloc(position_accessor->count * sizeof(Vertex));

		// required in specification, 3 floats;
		const float *min = position_accessor->min;
		const float *max = position_accessor->max;

		if (min == nullptr || max == nullptr)
			throw std::runtime_error("Position min or max value is NULL!");

		size[0] = max[0] - min[0];
		size[1] = max[1] - min[1];
		size[2] = max[2] - min[2];

		offset[0] = min[0];
		offset[1] = min[1];
		offset[2] = min[2];
	}

	for (size_t attribute_index = 0; attribute_index < cgltf_primitive.attributes_count; attribute_index++) {
		const cgltf_attribute &attribute = cgltf_primitive.attributes[attribute_index];

		const cgltf_accessor *accessor = cgltf_primitive.attributes[attribute_index].data;
		const cgltf_buffer_view *buffer_view = accessor->buffer_view;
		const uint8_t *buffer = (uint8_t *)buffer_view->buffer->data;

		const size_t begin = buffer_view->offset;
		const size_t end = begin + buffer_view->size;

		if (attribute.type == cgltf_attribute_type_position) {
			// position is always 3 floats in specification
			const size_t element_size = sizeof(float) * 3;

			size_t i = 0;
			for (size_t offset = begin; offset < end; offset += element_size) {
				memcpy(vertices[i].position, &buffer[offset], element_size);
				i++;
			}

			continue;
		}

		if (attribute.type == cgltf_attribute_type_normal) {
			// normal is always 3 floats in specification
			const size_t element_size = sizeof(float) * 3;

			size_t i = 0;
			for (size_t offset = begin; offset < end; offset += element_size) {
				memcpy(vertices[i].normal, &buffer[offset], element_size);
				i++;
			}

			continue;
		}

		if (attribute.type == cgltf_attribute_type_texcoord) {
			if (strncmp("TEXCOORD_0", attribute.name, strlen("TEXCOORD_0")) != 0)
				continue;

			// possible component types: unorm8, unorm16, float
			const cgltf_component_type component_type = accessor->component_type;
			const size_t element_size = cgltf_component_size(component_type) * 2;

			size_t i = 0;
			for (size_t offset = begin; offset < end; offset += element_size) {
				if (component_type == cgltf_component_type_r_32f) {
					const float *data = (float *)&buffer[offset];
					vertices[i].tex_coord[0] = data[0];
					vertices[i].tex_coord[1] = data[1];
				} else if (component_type == cgltf_component_type_r_16u) {
					const uint16_t *data = (uint16_t *)&buffer[offset];
					vertices[i].tex_coord[0] = data[0] * ONE_DIV_UINT16_MAX;
					vertices[i].tex_coord[1] = data[1] * ONE_DIV_UINT16_MAX;
				} else if (component_type == cgltf_component_type_r_8u) {
					const uint8_t *data = &buffer[offset];
					vertices[i].tex_coord[0] = data[0] * ONE_DIV_UINT8_MAX;
					vertices[i].tex_coord[1] = data[1] * ONE_DIV_UINT8_MAX;
				} else {
					throw std::runtime_error("Invalid texture coordinate component type!");
				}

				i++;
			}

			continue;
		}
	}

	_tangents_calculate(indices, index_count, vertices, vertex_count);

	float inverse_size[3] = {
		1.0f / size[0],
		1.0f / size[1],
		1.0f / size[2],
	};

	PackedVertex *packed_vertices = (PackedVertex *)malloc(vertex_count * sizeof(PackedVertex));

	for (size_t i = 0; i < vertex_count; i++) {
		packed_vertices[i] = _vertex_compress(vertices[i], inverse_size, offset);
	}

	free(vertices);

	Buffer index_buffer = {};
	index_buffer.data = indices;
	index_buffer.size = index_count * sizeof(uint32_t);

	Buffer vertex_buffer = {};
	vertex_buffer.data = packed_vertices;
	vertex_buffer.size = vertex_count * sizeof(PackedVertex);

	Primitive primitive = {};
	primitive.index_buffer = index_buffer;
	primitive.vertex_buffer = vertex_buffer;

	memcpy(primitive.size, size, sizeof(float) * 3);
	memcpy(primitive.offset, offset, sizeof(float) * 3);

	const cgltf_material *cgltf_material = cgltf_primitive.material;
	primitive.material_index = cgltf_material != nullptr ? cgltf_material_index(cgltf_data, cgltf_material) : 0;
	primitive.has_material = cgltf_material != nullptr;

	return primitive;
}

Node Loader::_node_load(const cgltf_data *cgltf_data, const cgltf_node &cgltf_node) {
	const cgltf_mesh *cgltf_mesh = cgltf_node.mesh;

	mat4 transform(1.0f);
	if (cgltf_node.has_matrix)
		memcpy(&transform, cgltf_node.matrix, sizeof(mat4));

	Node node = {};
	node.mesh_index = cgltf_mesh != nullptr ? cgltf_mesh_index(cgltf_data, cgltf_mesh) : 0;
	node.has_mesh = cgltf_mesh != nullptr;

	node.name = cgltf_node.name;

	memcpy(node.transform, &transform, sizeof(float) * 16);

	return node;
}

Mesh Loader::_mesh_load(const cgltf_data *cgltf_data, const cgltf_mesh &cgltf_mesh) {
	Primitive *primitives = (Primitive *)malloc(cgltf_mesh.primitives_count * sizeof(Primitive));
	size_t primitive_count = cgltf_mesh.primitives_count;

	for (size_t i = 0; i < primitive_count; i++)
		primitives[i] = _primitive_load(cgltf_data, cgltf_mesh.primitives[i]);

	Mesh mesh = {};
	mesh.primitives = primitives;
	mesh.primitive_count = primitive_count;

	return mesh;
}

int Loader::scene_load(const char *path, Scene *scene) {
	cgltf_options cgltf_options = {};
	cgltf_data *cgltf_data = nullptr;

	if (cgltf_parse_file(&cgltf_options, path, &cgltf_data) != cgltf_result_success)
		return FAILURE;

	if (cgltf_load_buffers(&cgltf_options, cgltf_data, path) != cgltf_result_success)
		return FAILURE;

	if (scene == nullptr)
		scene = new Scene;

	size_t node_count = cgltf_data->nodes_count;
	size_t mesh_count = cgltf_data->meshes_count;

	Node *nodes = new Node[node_count];
	Mesh *meshes = new Mesh[mesh_count];

	for (size_t node_index = 0; node_index < node_count; node_index++) {
		nodes[node_index] = _node_load(cgltf_data, cgltf_data->nodes[node_index]);
	}

	for (size_t mesh_index = 0; mesh_index < mesh_count; mesh_index++) {
		meshes[mesh_index] = _mesh_load(cgltf_data, cgltf_data->meshes[mesh_index]);
	}

	scene->nodes = nodes;
	scene->node_count = node_count;

	scene->meshes = meshes;
	scene->mesh_count = mesh_count;

	cgltf_free(cgltf_data);
	return SUCCESS;
}
