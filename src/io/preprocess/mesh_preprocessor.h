#ifndef MESH_PREPROCESSOR_H
#define MESH_PREPROCESSOR_H

#include <io/types/mesh.h>
#include <io/types/vertex.h>

namespace math {
struct vec3;
}

class MeshPreprocessor {
public:
	static void process(Mesh *mesh);
};

#endif // !MESH_PREPROCESSOR_H
