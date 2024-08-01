#ifndef MESH_H
#define MESH_H

#include <cstddef>
#include <cstdint>

#include "aabb.h"
#include "vertex.h"

typedef struct {
	uint32_t *data;
	uint32_t count;
} IndexArray;

typedef struct {
	Vertex *data;
	uint32_t count;
} VertexArray;

typedef struct {
	AABB aabb;
	size_t materialIndex;

	IndexArray indices;
	VertexArray vertices;
} Primitive;

typedef struct {
	Primitive *primitives;
	uint32_t primitiveCount;
} Mesh;

#endif // !MESH_H
