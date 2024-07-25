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

enum SamplerMode {
	SAMPLER_CLAMP_NEAREST,
	SAMPLER_CLAMP_LINEAR,
	SAMPLER_REPEAT_NEAREST,
	SAMPLER_REPEAT_LINEAR,
	SAMPLER_CLAMP_NEAREST_MIPMAPS,
	SAMPLER_CLAMP_LINEAR_MIPMAPS,
	SAMPLER_REPEAT_NEAREST_MIPMAPS,
	SAMPLER_REPEAT_LINEAR_MIPMAPS,
};

typedef struct {
	float factor[3];
	uint64_t texture;
} EmissionData;

typedef struct {
	SamplerMode samplerMode;
	AlphaMode alphaMode;
	CullMode cullMode;
	bool unlit;

	float albedoFactor[3];
	uint64_t albedoTexture;

	float occlusionFactor;
	float roughnessFactor;
	float metallicFactor;
	uint64_t occlusionRoughnessMetallicTexture;

	// specialization
	EmissionData *emissionData;
} Material;

#endif // !MATERIAL_H
