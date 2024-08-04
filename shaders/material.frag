#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inTangent;
layout(location = 2) in vec3 inBitangent;
layout(location = 3) in vec3 inNormal;
layout(location = 4) in vec2 inTexCoord;

layout(location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform SceneUBO {
	vec3 CAMERA_POSITION;
	float TIME;
};

const float PI = 3.14159265359;

// layout(early_fragment_tests) in;

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return (1.0 - F0) * pow(1.0 - cosTheta, 5.0) + F0;
}

float distributionGGX(float NdotH, float roughness) {
	// a = roughness * roughness
	// a2 = a * a
	float a2 = (roughness * roughness) * (roughness * roughness);
	float x = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
	return a2 / ((x * x) * PI);
}

float geometrySchlickGGX(float NdotV, float roughness) {
	float r = roughness + 1.0;
	float k = (r * r) * 0.125;
	return NdotV / (NdotV * (1.0 - k) + k);
}

float geometrySmith(float NdotV, float NdotL, float roughness) {
	return geometrySchlickGGX(NdotV, roughness) * geometrySchlickGGX(NdotL, roughness);
}

vec3 cookTorranceBRDF(float NdotV, float NdotL, float NdotH, vec3 fresnel, float roughness) {
	vec3 num = distributionGGX(NdotH, roughness) * geometrySmith(NdotV, NdotL, roughness) * fresnel;
	float denom = 4.0 * (NdotV * NdotL) + 0.0001;
	return num * (1.0 / denom);
}

vec3 lightCalculate(vec3 N, vec3 V, vec3 L, float roughness, vec3 radiance, vec3 albedo, float metallic) {
	vec3 H = normalize(V + L);

	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float NdotH = max(dot(N, H), 0.0);
	float cosTheta = max(dot(H, V), 0.0);

	vec3 F0 = mix(vec3(0.04), albedo, metallic);
	vec3 fresnel = fresnelSchlick(cosTheta, F0);
	
	vec3 specular = cookTorranceBRDF(NdotV, NdotL, NdotH, fresnel, roughness);

	vec3 kS = fresnel;
	vec3 kD = vec3(1.0) - kS;
	kD *= 1.0 - metallic;

	return (kD * albedo / PI + specular) * radiance * NdotL;
}

void main() {
	vec3 albedo = vec3(1.0);
	float roughness = 1.0;
	float metallic = 0.0;

	vec3 N = inNormal;
	vec3 V = normalize(CAMERA_POSITION - inPosition);
	vec3 L = normalize(vec3(1.0));

	vec3 radiance = vec3(1.0);
	vec3 outLight = lightCalculate(N, V, L, roughness, radiance, albedo, metallic);

	vec3 ambient = vec3(0.04);
	vec3 color = ambient + outLight;
	outFragColor = vec4(color, 1.0);
}
