#ifndef SCENE_H
#define SCENE_H

#include <cstddef>

typedef struct Node Node;
typedef struct Mesh Mesh;

typedef struct Scene {
	Node *nodes;
	size_t node_count;

	Mesh *meshes;
	size_t mesh_count;
} Scene;

#endif // !SCENE_H
