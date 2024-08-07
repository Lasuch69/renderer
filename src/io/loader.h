#ifndef LOADER_H
#define LOADER_H

#include <cstddef>
#include <cstdint>

typedef struct Scene Scene;

typedef struct Vertex Vertex;
typedef struct PackedVertex PackedVertex;

typedef struct Node Node;
typedef struct Primitive Primitive;
typedef struct Mesh Mesh;

#define SUCCESS 0
#define FAILURE 0

class Loader {
private:
	typedef struct cgltf_data cgltf_data;
	typedef struct cgltf_node cgltf_node;
	typedef struct cgltf_primitive cgltf_primitive;
	typedef struct cgltf_mesh cgltf_mesh;

	static bool _tangents_calculate(const uint32_t *indices, size_t index_count, Vertex *vertices, size_t vertex_count);
	static PackedVertex _vertex_compress(const Vertex &vertex, const float *inverse_size, const float *offset);

	static Primitive _primitive_load(const cgltf_data *gltf_data, const cgltf_primitive &cgltf_primitive);

	static Node _node_load(const cgltf_data *cgltf_data, const cgltf_node &cgltf_node);
	static Mesh _mesh_load(const cgltf_data *cgltf_data, const cgltf_mesh &cgltf_mesh);

public:
	static int scene_load(const char *path, Scene *scene);
};

#endif // !LOADER_H
