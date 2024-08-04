#ifndef MESH_H
#define MESH_H

#include <cstddef>
#include <cstdint>

#include "vertex.h"

typedef struct IndexArray {
	uint32_t *data;
	uint32_t count;
} IndexArray;

typedef struct VertexArray {
	Vertex *data;
	uint32_t count;
} VertexArray;

typedef struct Primitive {
	float size[3];
	float offset[3];

	IndexArray indices;
	VertexArray vertices;

	size_t materialIndex;
} Primitive;

typedef struct Mesh {
	Primitive *primitives;
	uint32_t primitiveCount;
} Mesh;

#endif // !MESH_H
