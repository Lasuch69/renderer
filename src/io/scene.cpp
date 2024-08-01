#include <cstdlib>

#include "scene.h"

Node *Scene::nodes() const {
	return m_nodes;
}

size_t Scene::nodeCount() const {
	return m_nodeCount;
}

Mesh *Scene::meshes() const {
	return m_meshes;
}

size_t Scene::meshCount() const {
	return m_meshCount;
}

Scene::Scene(Node *_nodes, size_t _nodeCount, Mesh *_meshes, size_t _meshCount) {
	m_nodes = _nodes;
	m_nodeCount = _nodeCount;

	m_meshes = _meshes;
	m_meshCount = _meshCount;
}

Scene::~Scene() {
	free(m_nodes);
	free(m_meshes);
}
