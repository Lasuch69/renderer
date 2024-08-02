#ifndef MESH_H
#define MESH_H

#include <cstddef>
#include <cstdint>

#include "vertex.h"

typedef struct {
	float x, y, z;
	float w, h, d;
} AABB;

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
	IndexArray indices;
	VertexArray vertices;

	size_t materialIndex;
} Primitive;

typedef struct {
	Primitive *primitives;
	uint32_t primitiveCount;
} Mesh;

#endif // !MESH_H
