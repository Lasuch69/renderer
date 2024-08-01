#ifndef NODE_H
#define NODE_H

#include <cstddef>
#include <math/types/mat4.h>

typedef struct {
	bool hasMesh;
	size_t meshIndex;

	math::mat4 transform;
} Node;

#endif // !NODE_H
