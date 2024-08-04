#ifndef MESH_STORAGE_H
#define MESH_STORAGE_H

#include <cstddef>
#include <cstdint>

#include <resource_owner.h>

typedef size_t BufferID;
typedef size_t MaterialID;

typedef size_t MeshID;
typedef size_t MeshInstanceID;

typedef struct Mesh Mesh;

namespace math {
struct mat4;
};

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

		BufferID indexBuffer;
		BufferID vertexBuffer;
		uint32_t indexCount;
	} PrimitiveRD;

	typedef struct MeshRD {
		// INFO: used for frustum culling
		AABB aabb;
		PrimitiveRD *primitives;
		uint32_t primitiveCount;
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

	ResourceOwner<MeshRD> m_meshOwner;
	ResourceOwner<MeshInstanceRD> m_meshInstanceOwner;

public:
	MeshID meshCreate(const Mesh &mesh);
	void meshDestroy(MeshID meshID);

	MeshInstanceID meshInstanceCreate();
	void meshInstanceSetMesh(MeshInstanceID meshInstanceID, MeshID meshID);
	void meshInstanceSetTransform(MeshInstanceID meshInstanceID, const math::mat4 &transform);
	void meshInstanceDestroy(MeshInstanceID meshInstanceID);
};

#endif // !MESH_STORAGE_H
