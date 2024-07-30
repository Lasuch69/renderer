#version 450

// #extension GL_EXT_nonuniform_qualifier : enable
// #extension GL_GOOGLE_include_directive : enable

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec3 fragTangent;
layout(location = 2) in vec3 fragBitangent;
layout(location = 3) in vec3 fragNormal;
layout(location = 4) in vec2 fragTexCoord;

// layout(location = 5) in flat uint instanceIndex;

layout(location = 0) out vec3 outFragAlbedo;
layout(location = 1) out vec3 outFragNormal;
layout(location = 2) out vec2 outFragRoughnessMetallic;

// layout(set = 0, binding = 0) uniform sampler SAMPLER[8];
// layout(set = 0, binding = 1) uniform texture2D TEXTURE[];

// #include "include/material_incl.glsl"

// layout(set = 1, binding = 0) readonly buffer MaterialSSBO {
// 	Material materials[];
// };

vec3 unpackNormal(vec2 rg, mat3 tbn) {
	rg = rg * 2.0 - vec2(1.0);
	float b = sqrt(1.0 - clamp(dot(rg, rg), 0.0, 1.0));

	vec3 n = vec3(rg, b);
	return normalize(tbn * n);
}

void main() {
	// Material material = materials[instanceIndex];
	mat3 TBN = mat3(fragTangent, fragBitangent, fragNormal);

	vec3 albedo = vec3(1.0);
	vec3 normal = vec3(0.5, 0.5, 0.0);
	vec2 roughnessMetallic = vec2(1.0, 0.0);

	// vec3 albedo = texture(sampler2D(TEXTURE[nonuniformEXT(material.albedoTexture)], SAMPLER[material.samplerMode]), fragTexCoord).rgb;
	// vec3 normal = texture(sampler2D(TEXTURE[nonuniformEXT(material.normalTexture)], SAMPLER[material.samplerMode]), fragTexCoord).rgb;
	// vec2 roughnessMetallic = texture(sampler2D(TEXTURE[nonuniformEXT(material.occlusionRoughnessMetallicTexture)], SAMPLER[material.samplerMode]), fragTexCoord).gb;

	// albedo *= material.albedoFactor;
	normal = unpackNormal(normal.rg, TBN);
	
	// roughnessMetallic.r *= material.roughnessFactor;
	// roughnessMetallic.g *= material.metallicFactor;

	outFragAlbedo = albedo;
	outFragNormal = normal;
	outFragRoughnessMetallic = roughnessMetallic;
}
