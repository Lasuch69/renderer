#ifndef NODE_H
#define NODE_H

#include <cstddef>

typedef struct Node {
	size_t mesh_index;
	bool has_mesh;

	char *name;
	float transform[16];
} Node;

#endif // !NODE_H
