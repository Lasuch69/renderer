#ifndef MESH_H
#define MESH_H

#include <cstdint>

typedef struct IndexArray {
	uint32_t *data;
	uint32_t count;
} IndexArray;

typedef struct VertexArray {
	uint8_t *data;
	uint32_t count;
	bool isPacked;
} VertexArray;

typedef struct Primitive {
	float size[3];
	float offset[3];

	IndexArray indices;
	VertexArray vertices;

	bool hasMaterial;
	uint64_t materialIndex;
} Primitive;

typedef struct Mesh {
	Primitive *primitives;
	uint32_t primitiveCount;
} Mesh;

#endif // !MESH_H
