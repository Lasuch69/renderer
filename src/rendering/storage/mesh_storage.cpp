#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <io/types/mesh.h>
#include <io/types/vertex.h>
#include <math/types/mat4.h>

#include <rendering/device/rendering_device.h>
#include <rendering/typedefs.h>

#include "mesh_storage.h"

#define CHECK(_expr, msg)                                                                                              \
	if (_expr) {                                                                                                       \
		fprintf(stderr, "%s\n", msg);                                                                                  \
		return;                                                                                                        \
	}

inline float min(float a, float b) {
	return a < b ? a : b;
}

inline float max(float a, float b) {
	return a > b ? a : b;
}

MeshID MeshStorage::mesh_create(const Mesh &mesh) {
	MeshRD mesh_rd;
	mesh_rd.primitives = new PrimitiveRD[mesh.primitive_count];
	mesh_rd.primitive_count = mesh.primitive_count;

	RD &rd = RD::singleton();

	for (uint32_t i = 0; i < mesh.primitive_count; i++) {
		const Buffer &index_buffer = mesh.primitives[i].index_buffer;
		const Buffer &vertex_buffer = mesh.primitives[i].vertex_buffer;

		mesh_rd.primitives[i].index_buffer = rd.index_buffer_create(index_buffer.data, index_buffer.size);
		mesh_rd.primitives[i].vertex_buffer = rd.vertex_buffer_create(vertex_buffer.data, vertex_buffer.size);
		mesh_rd.primitives[i].index_count = index_buffer.size / sizeof(uint32_t);
	}

	return m_mesh_owner.insert(mesh_rd);
}

void MeshStorage::mesh_destroy(MeshID mesh_id) {
	MeshRD *mesh_rd = m_mesh_owner.get(mesh_id);
	if (mesh_rd == nullptr)
		return;

	for (uint32_t i = 0; i < mesh_rd->primitive_count; i++) {
		PrimitiveRD &primitive = mesh_rd->primitives[i];
		RD::singleton().buffer_destroy(primitive.index_buffer);
		RD::singleton().buffer_destroy(primitive.vertex_buffer);
	}

	free(mesh_rd->primitives);
	m_mesh_owner.remove(mesh_id);
}

MeshInstanceID MeshStorage::mesh_instance_create() {
	return m_mesh_instance_owner.insert({});
}

void MeshStorage::mesh_instance_set_mesh(MeshInstanceID mesh_instance_id, MeshID mesh_id) {
	CHECK(m_mesh_instance_owner.has(mesh_instance_id) == false, "MeshInstanceID invalid!");
	m_mesh_instance_owner.get(mesh_instance_id)->mesh = mesh_id;
}

void MeshStorage::mesh_instance_set_transform(MeshInstanceID mesh_instance_id, const math::mat4 &transform) {
	CHECK(m_mesh_instance_owner.has(mesh_instance_id) == false, "MeshInstanceID invalid!");
	memcpy(m_mesh_instance_owner.get(mesh_instance_id), &transform, sizeof(transform));
}

void MeshStorage::mesh_instance_destroy(MeshInstanceID mesh_instance_id) {
	m_mesh_instance_owner.remove(mesh_instance_id);
}

MeshRD **MeshStorage::mesh_list_get(size_t *size) const {
	return m_mesh_owner.list(size);
}

MeshInstanceRD **MeshStorage::mesh_instance_list_get(size_t *size) const {
	return m_mesh_instance_owner.list(size);
}
