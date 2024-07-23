#ifndef GLTF_LOADER_H
#define GLTF_LOADER_H

#include <cstdint>

#include <io/types/mesh.h>
#include <io/types/vertex.h>

#include <math/types/mat4.h>

struct cgltf_attribute;
struct cgltf_mesh;
struct cgltf_node;

typedef struct {
	char *name;
	uint32_t *meshIndex;
	math::mat4 transform;
} Node;

class GLTFLoader {
private:
	static bool _checkAttributes(const cgltf_attribute *attributes, uint32_t attributeCount);
	static math::mat4 _extractTransform(const cgltf_node &node);
	static void _tangentsGenerate(const IndexArray &indices, VertexArray &vertices);

	static Mesh _meshLoad(const cgltf_mesh &mesh);
	static Node _nodeLoad(const cgltf_node &node);

public:
	static void loadFile(const char *path);
};

#endif // !GLTF_LOADER_H
