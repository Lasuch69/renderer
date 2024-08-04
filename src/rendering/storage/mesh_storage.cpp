#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <common/mesh.h>
#include <math/types/mat4.h>

#include <rendering/device/rendering_device.h>

#include "mesh_storage.h"

#define CHECK(_expr, msg)                                                                                              \
	if (_expr) {                                                                                                       \
		fprintf(stderr, "%s\n", msg);                                                                                  \
		return;                                                                                                        \
	}

MeshID MeshStorage::meshCreate(const Mesh &mesh) {
	MeshRD meshRD;
	meshRD.primitives = new PrimitiveRD[mesh.primitiveCount];
	meshRD.primitiveCount = mesh.primitiveCount;

	RD &rd = RD::singleton();

	for (uint32_t i = 0; i < mesh.primitiveCount; i++) {
		const IndexArray &indices = mesh.primitives[i].indices;
		const VertexArray &vertices = mesh.primitives[i].vertices;

		meshRD.primitives[i].indexBuffer = rd.indexBufferCreate(indices.data, indices.count * sizeof(uint32_t));
		meshRD.primitives[i].vertexBuffer = rd.vertexBufferCreate(vertices.data, vertices.count * sizeof(Vertex));
		meshRD.primitives[i].indexCount = indices.count;

		memcpy(meshRD.primitives[i].aabb.size, mesh.primitives[i].size, sizeof(float) * 3);
		memcpy(meshRD.primitives[i].aabb.offset, mesh.primitives[i].offset, sizeof(float) * 3);

		const float *size = mesh.primitives[i].size;
		const float *offset = mesh.primitives[i].offset;

		meshRD.aabb.size[0] = meshRD.aabb.size[0] > size[0] ? meshRD.aabb.size[0] : size[0];
		meshRD.aabb.size[1] = meshRD.aabb.size[1] > size[1] ? meshRD.aabb.size[1] : size[1];
		meshRD.aabb.size[2] = meshRD.aabb.size[2] > size[2] ? meshRD.aabb.size[2] : size[2];

		meshRD.aabb.offset[0] = meshRD.aabb.offset[0] < offset[0] ? meshRD.aabb.offset[0] : offset[0];
		meshRD.aabb.offset[1] = meshRD.aabb.offset[1] < offset[1] ? meshRD.aabb.offset[1] : offset[1];
		meshRD.aabb.offset[2] = meshRD.aabb.offset[2] < offset[2] ? meshRD.aabb.offset[2] : offset[2];
	}

	return m_meshOwner.insert(meshRD);
}

void MeshStorage::meshDestroy(MeshID meshID) {
	MeshRD *meshRD = m_meshOwner.get(meshID);
	if (meshRD == nullptr)
		return;

	for (uint32_t i = 0; i < meshRD->primitiveCount; i++) {
		PrimitiveRD &primitive = meshRD->primitives[i];
		RD::singleton().bufferDestroy(primitive.indexBuffer);
		RD::singleton().bufferDestroy(primitive.vertexBuffer);
	}

	free(meshRD->primitives);
	m_meshOwner.remove(meshID);
}

MeshInstanceID MeshStorage::meshInstanceCreate() {
	return m_meshInstanceOwner.insert({});
}

void MeshStorage::meshInstanceSetMesh(MeshInstanceID meshInstanceID, MeshID meshID) {
	CHECK(m_meshInstanceOwner.has(meshInstanceID) == false, "MeshInstanceID invalid!");
	m_meshInstanceOwner.get(meshInstanceID)->mesh = meshID;
}

void MeshStorage::meshInstanceSetTransform(MeshInstanceID meshInstanceID, const math::mat4 &transform) {
	CHECK(m_meshInstanceOwner.has(meshInstanceID) == false, "MeshInstanceID invalid!");
	memcpy(m_meshInstanceOwner.get(meshInstanceID), &transform, sizeof(transform));
}

void MeshStorage::meshInstanceDestroy(MeshInstanceID meshInstanceID) {
	m_meshInstanceOwner.remove(meshInstanceID);
}
