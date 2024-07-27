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
	AABB aabb;
	uint32_t material;

	VertexArray vertexArray;
	IndexArray indexArray;
} Primitive;

typedef struct {
	Primitive *primitives;
	uint32_t primitiveCount;
} Mesh;

#endif // !MESH_H
