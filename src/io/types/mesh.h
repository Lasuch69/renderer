#ifndef MESH_H
#define MESH_H

#include <cstddef>

typedef struct Buffer {
	void *data;
	size_t size;
} Buffer;

typedef struct Primitive {
	Buffer index_buffer;
	Buffer vertex_buffer;

	float size[3];
	float offset[3];

	size_t material_index;
	bool has_material;
} Primitive;

typedef struct Mesh {
	Primitive *primitives;
	size_t primitive_count;
} Mesh;

#endif // !MESH_H
