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

float distributionGGX(float NdotH, float R) {
	float a = R * R;
	float a2 = a * a;

	float num = a2;
	float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
	denom = PI * denom * denom;
	return num / denom;
}

float geometrySchlickGGX(float NdotV, float R) {
	float r = (R + 1.0);
	float k = (r * r) / 8.0;

	float num = NdotV;
	float denom = NdotV * (1.0 - k) + k;
	return num / denom;
}

float geometrySmith(float NdotV, float NdotL, float R) {
	float ggx2 = geometrySchlickGGX(NdotV, R);
	float ggx1 = geometrySchlickGGX(NdotL, R);
	return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
	return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

vec3 cookTorranceBRDF(float NdotV, float NdotL, float NdotH, vec3 F, float R) {
	float distribution = distributionGGX(NdotH, R);
	float geometrySmith = geometrySmith(NdotV, NdotL, R);

	vec3 numerator = distribution * geometrySmith * F;
	float denominator = 4.0 * NdotV * NdotL + 0.0001;
	return numerator / denominator;
}

vec3 lightCalculate(vec3 N, vec3 V, vec3 L, float R, vec3 radiance, vec3 albedo, float metallic) {
	vec3 H = normalize(V + L);

	float NdotV = max(dot(N, V), 0.0);
	float NdotL = max(dot(N, L), 0.0);
	float NdotH = max(dot(N, H), 0.0);
	float cosTheta = max(dot(H, V), 0.0);

	vec3 F0 = vec3(0.04);
	F0 = mix(F0, albedo, metallic);

	vec3 F = fresnelSchlick(cosTheta, F0);
	vec3 specular = cookTorranceBRDF(NdotV, NdotL, NdotH, F, R);

	vec3 kS = F;
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
