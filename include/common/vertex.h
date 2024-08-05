#ifndef VERTEX_H
#define VERTEX_H

#include <cstdint>

typedef struct Vertex {
	float position[3];
	float normal[3];
	float tangent[3];
	float texCoord[2];
} Vertex;

typedef struct PackedVertex {
	uint16_t position[3];
	uint8_t tangent[3];
	uint8_t normal[3];
	uint8_t texCoord[3];
	uint8_t texOffset;
} PackedVertex;

#endif // !VERTEX_H
