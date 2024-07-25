#ifndef MESH_STORAGE_H
#define MESH_STORAGE_H

#include <cstdint>

#include "rendering/types/allocated.h"

typedef struct {
	AllocatedBuffer vertexBuffer;
	AllocatedBuffer indexBuffer;
	uint32_t indexCount;
} Mesh;

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
