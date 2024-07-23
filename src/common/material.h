#ifndef MATERIAL_H
#define MATERIAL_H

#include <cstdint>

enum AlphaMode {
	ALPHA_OPAQUE,
	ALPHA_BLEND,
};

enum CullMode {
	CULL_BACK,
	CULL_FRONT,
	CULL_DISABLED,
};

typedef struct {
	AlphaMode alphaMode;
	CullMode cullMode;
	bool unlit;
} ShadingMode;

typedef struct {
	float albedoFactor[3];
	uint64_t albedoTexture;

	float occlusionFactor;
	float roughnessFactor;
	float metallicFactor;
	uint64_t occlusionRoughnessMetallicTexture;

	float emissionFactor[3];
	uint64_t emissionTexture;
} Material;

#endif // !MATERIAL_H
