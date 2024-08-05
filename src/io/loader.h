#ifndef LOADER_H
#define LOADER_H

typedef struct Scene Scene;

namespace Loader {

Scene *sceneLoadGlTF(const char *path);

} // namespace Loader

#endif // !LOADER_H
