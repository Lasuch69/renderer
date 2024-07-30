struct Material {
	vec3 albedoFactor;
	uint albedoTexture;
	
	float occlusionFactor;
	float roughnessFactor;
	float metallicFactor;
	uint occlusionRoughnessMetallicTexture;

	uint normalTexture;
	uint samplerMode;

	float _padding0;
	float _padding1;
};
