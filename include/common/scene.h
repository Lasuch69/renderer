#ifndef SCENE_H
#define SCENE_H

#include <cstdint>

#include "mesh.h"
#include "node.h"

typedef struct Scene {
	Node *nodes;
	uint64_t nodeCount;

	Mesh *meshes;
	uint64_t meshCount;
} Scene;

#endif // !SCENE_H
