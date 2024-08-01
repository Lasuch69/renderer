#ifndef GLTF_LOADER_H
#define GLTF_LOADER_H

#include <io/scene.h>

class GLTFLoader {
public:
	static Scene *loadFile(const char *path);
};

#endif // !GLTF_LOADER_H
