#ifndef MESH_STORAGE_H
#define MESH_STORAGE_H

#include <cstddef>
#include <cstdint>

#include "common/aabb.h"
#include "rendering/types/allocated.h"

typedef size_t RID;

typedef struct {
	AABB aabb;
	RID material;

	AllocatedBuffer vertexBuffer;
	AllocatedBuffer indexBuffer;
	uint32_t indexCount;

	// INFO: supply hash from vertices used to create vertexBuffer
	uint64_t hash;
} PrimitiveRD;

typedef struct {
	uint32_t primitiveCount;
	PrimitiveRD *primitives;
} MeshRD;

typedef struct {
	float transform[16];
	RID mesh;
} MeshInstanceRD;

class MeshStorage {
public:
	static MeshStorage &singleton() {
		static MeshStorage instance;
		return instance;
	}

	MeshStorage(MeshStorage const &) = delete;
	void operator=(MeshStorage const &) = delete;

private:
	MeshStorage() {}

public:
};

#endif // !MESH_STORAGE_H
