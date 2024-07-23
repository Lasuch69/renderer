#ifndef MESH_H
#define MESH_H

#include <cstdint>

#include "aabb.h"
#include "vertex.h"

typedef struct {
	Vertex *vertices;
	uint32_t vertexCount;
} VertexArray;

typedef struct {
	uint32_t *indices;
	uint32_t indexCount;
} IndexArray;

typedef struct {
	VertexArray vertexArray;
	IndexArray indexArray;
	uint64_t materialIndex;
} Primitive;

typedef struct {
	Primitive *primitives;
	uint32_t primitveCount;
	AABB aabb;
} Mesh;

#endif // !MESH_H
