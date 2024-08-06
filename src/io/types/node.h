#ifndef NODE_H
#define NODE_H

#include <cstdint>

typedef struct {
	bool hasMesh;
	uint64_t meshIndex;
	float transform[16];
} Node;

#endif // !NODE_H
