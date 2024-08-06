#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include <meshoptimizer/meshoptimizer.h>

#include <io/types/mesh.h>
#include <io/types/vertex.h>
#include <math/types/vec3.h>

#include "encoding.h"
#include "mesh_preprocessor.h"

#define UINT4_MAX 0xf

using namespace encoding;

inline uint32_t min(uint32_t a, uint32_t b) {
	return a < b ? a : b;
}

bool vertexArrayPack(VertexArray &vertexArray, const math::vec3 &size, const math::vec3 &offset) {
	PackedVertex *packedVertices = (PackedVertex *)malloc(vertexArray.count * sizeof(PackedVertex));

	const math::vec3 inverseSize = {
		1.0f / size.x,
		1.0f / size.y,
		1.0f / size.z,
	};

	{
		const Vertex *vertices = (Vertex *)vertexArray.data;

		for (uint32_t i = 0; i < vertexArray.count; i++) {
			const Vertex &v = vertices[i];
			PackedVertex &p = packedVertices[i];

			// pack position
			p.position[0] = ((v.position[0] - offset.x) * inverseSize.x) * UINT16_MAX;
			p.position[1] = ((v.position[1] - offset.y) * inverseSize.y) * UINT16_MAX;
			p.position[2] = ((v.position[2] - offset.z) * inverseSize.z) * UINT16_MAX;

			// pack tangents
			math::vec3 t = { v.tangent[0], v.tangent[1], v.tangent[2] };
			t = math::normalize(t);

			float32x2 tOct = float32x3ToOct(t.x, t.y, t.z);
			tOct.x = (tOct.x * 0.5f) + 0.5f;
			tOct.y = (tOct.y * 0.5f) + 0.5f;

			unorm12x2 tangent12x2 = float32x2ToUnorm12x2(tOct);
			p.tangent[0] = tangent12x2.data[0];
			p.tangent[1] = tangent12x2.data[1];
			p.tangent[2] = tangent12x2.data[2];

			// pack normals
			math::vec3 n = { v.normal[0], v.normal[1], v.normal[2] };
			n = math::normalize(n);

			float32x2 normalOct = float32x3ToOct(n.x, n.y, n.z);
			normalOct.x = (normalOct.x * 0.5f) + 0.5f;
			normalOct.y = (normalOct.y * 0.5f) + 0.5f;

			unorm12x2 normal12x2 = float32x2ToUnorm12x2(normalOct);
			p.normal[0] = normal12x2.data[0];
			p.normal[1] = normal12x2.data[1];
			p.normal[2] = normal12x2.data[2];

			// pack texCoord
			float32x2 texCoord = {
				v.texCoord[0] - int(v.texCoord[0]),
				v.texCoord[1] - int(v.texCoord[1]),
			};

			unorm12x2 texCoord12x2 = float32x2ToUnorm12x2(texCoord);
			p.texCoord[0] = texCoord12x2.data[0];
			p.texCoord[1] = texCoord12x2.data[1];
			p.texCoord[2] = texCoord12x2.data[2];

			// TODO: Handle negative offset
			uint8_t texOffsetX = min(int(v.texCoord[0]), UINT4_MAX);
			uint8_t texOffsetY = min(int(v.texCoord[1]), UINT4_MAX);

			// INFO: integer texture offset packed as uint4x2
			p.texOffset = (texOffsetY << 4) + texOffsetX;
		}
	}

	free(vertexArray.data);
	vertexArray.data = (uint8_t *)packedVertices;
	vertexArray.isPacked = true;

	return true;
}

bool remapIndices(IndexArray &indices, VertexArray &vertices) {
	size_t vertexSize = sizeof(Vertex);
	uint32_t *remap = (uint32_t *)malloc(indices.count * sizeof(uint32_t));

	size_t vertexCount =
			meshopt_generateVertexRemap(remap, indices.data, indices.count, vertices.data, vertices.count, vertexSize);

	uint32_t *remappedIndices = (uint32_t *)malloc(indices.count * sizeof(uint32_t));
	Vertex *remappedVertices = (Vertex *)malloc(vertexCount * sizeof(Vertex));

	meshopt_remapIndexBuffer(remappedIndices, indices.data, indices.count, remap);
	meshopt_remapVertexBuffer(remappedVertices, vertices.data, vertices.count, vertexSize, remap);

	// free useless data
	free(remap);
	free(indices.data);
	free(vertices.data);

	indices.data = remappedIndices;
	vertices.data = (uint8_t *)remappedVertices;
	vertices.count = vertexCount;

	return true;
}

bool primitiveProcess(Primitive &primitive) {
	IndexArray &indices = primitive.indices;
	VertexArray &vertices = primitive.vertices;
	remapIndices(indices, vertices);

	uint32_t indexCount = indices.count;
	uint32_t vertexCount = vertices.count;

	size_t vertexSize = sizeof(Vertex);
	meshopt_optimizeVertexCache(indices.data, indices.data, indexCount, vertexCount);
	meshopt_optimizeVertexFetch(vertices.data, indices.data, indexCount, vertices.data, vertexCount, vertexSize);

	const math::vec3 size = {
		primitive.size[0],
		primitive.size[1],
		primitive.size[2],
	};

	const math::vec3 offset = {
		primitive.offset[0],
		primitive.offset[1],
		primitive.offset[2],
	};

	vertexArrayPack(vertices, size, offset);

	return true;
}

void MeshPreprocessor::process(Mesh *mesh) {
	for (uint32_t i = 0; i < mesh->primitiveCount; i++) {
		primitiveProcess(mesh->primitives[i]);
	}
}
