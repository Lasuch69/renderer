#ifndef SCENE_H
#define SCENE_H

#include <cstddef>

#include <common/mesh.h>
#include <common/node.h>

class Scene {
private:
	Node *m_nodes;
	size_t m_nodeCount;

	Mesh *m_meshes;
	size_t m_meshCount;

public:
	Node *nodes() const;
	size_t nodeCount() const;

	Mesh *meshes() const;
	size_t meshCount() const;

	Scene() {}
	Scene(Node *_nodes, size_t _nodeCount, Mesh *_meshes, size_t _meshCount);
	~Scene();
};

#endif // !SCENE_H
