#ifndef MESH_STORAGE_H
#define MESH_STORAGE_H

#include <rendering/resource_owner.h>
#include <rendering/typedefs.h>

typedef struct Mesh Mesh;

namespace math {
struct mat4;
} // namespace math

class MeshStorage {
private:
	// Used in GPU buffers!
	typedef struct AABB {
		float size[3];
		float _pad0;
		float offset[3];
		float _pad1;
	} AABB;
	static_assert(sizeof(AABB) % 16 == 0, "AABB size is not multiple of 16!");

	typedef struct PrimitiveRD {
		// INFO: used for vertex decoding
		AABB aabb;
		MaterialID material;

		BufferID index_buffer;
		BufferID vertex_buffer;
		uint32_t index_count;
	} PrimitiveRD;

	typedef struct MeshRD {
		// INFO: used for frustum culling
		AABB aabb;
		PrimitiveRD *primitives;
		uint32_t primitive_count;
	} MeshRD;

	typedef struct MeshInstanceRD {
		MeshID mesh;
		float transform[16];
	} MeshInstanceRD;

public:
	static MeshStorage &singleton() {
		static MeshStorage instance;
		return instance;
	}

	MeshStorage(MeshStorage const &) = delete;
	void operator=(MeshStorage const &) = delete;

private:
	MeshStorage() {}

	ResourceOwner<MeshRD> m_mesh_owner;
	ResourceOwner<MeshInstanceRD> m_mesh_instance_owner;

public:
	MeshID mesh_create(const Mesh &mesh);
	void mesh_destroy(MeshID mesh_id);

	MeshInstanceID mesh_instance_create();
	void mesh_instance_set_mesh(MeshInstanceID mesh_instance_id, MeshID mesh_id);
	void mesh_instance_set_transform(MeshInstanceID mesh_instance_id, const math::mat4 &transform);
	void mesh_instance_destroy(MeshInstanceID meshInstance_id);
};

#endif // !MESH_STORAGE_H
