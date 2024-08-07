#version 450

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_tangent;
layout(location = 2) in vec3 in_bitangent;
layout(location = 3) in vec3 in_normal;
layout(location = 4) in vec2 in_tex_coord;

layout(location = 0) out vec4 out_frag_color;

layout(set = 0, binding = 0) uniform SceneUBO {
	vec3 CAMERA_POSITION;
	float TIME;
};

const float PI = 3.14159265359;

// layout(early_fragment_tests) in;

vec3 fresnel_schlick(float cos_theta, vec3 f0) {
	return (1.0 - f0) * pow(1.0 - cos_theta, 5.0) + f0;
}

float distribution_ggx(float n_dot_h, float roughness) {
	// a = roughness * roughness
	// a2 = a * a
	float a2 = (roughness * roughness) * (roughness * roughness);
	float x = (n_dot_h * n_dot_h) * (a2 - 1.0) + 1.0;
	return a2 / ((x * x) * PI);
}

float geometry_schlick_ggx(float n_dot_v, float roughness) {
	float r = roughness + 1.0;
	float k = (r * r) * 0.125;
	return n_dot_v / (n_dot_v * (1.0 - k) + k);
}

float geometry_smith(float n_dot_v, float n_dot_l, float roughness) {
	return geometry_schlick_ggx(n_dot_v, roughness) * geometry_schlick_ggx(n_dot_l, roughness);
}

vec3 cook_torrance_brdf(float n_dot_v, float n_dot_l, float n_dot_h, vec3 fresnel, float roughness) {
	vec3 num = distribution_ggx(n_dot_h, roughness) * geometry_smith(n_dot_v, n_dot_l, roughness) * fresnel;
	float denom = 4.0 * (n_dot_v * n_dot_l) + 0.0001;
	return num * (1.0 / denom);
}

vec3 light_calculate(vec3 normal, vec3 view, vec3 light, float roughness, vec3 radiance, vec3 albedo, float metallic) {
	vec3 half_vec = normalize(view + light);

	float n_dot_v = max(dot(normal, view), 0.0);
	float n_dot_l = max(dot(normal, light), 0.0);
	float n_dot_h = max(dot(normal, half_vec), 0.0);
	float cos_theta = max(dot(half_vec, view), 0.0);

	vec3 f0 = mix(vec3(0.04), albedo, metallic);
	vec3 fresnel = fresnel_schlick(cos_theta, f0);
	
	vec3 specular = cook_torrance_brdf(n_dot_v, n_dot_l, n_dot_h, fresnel, roughness);

	vec3 k_s = fresnel;
	vec3 k_d = vec3(1.0) - k_s;
	k_d *= 1.0 - metallic;

	return (k_d * albedo / PI + specular) * radiance * n_dot_l;
}

void main() {
	vec3 albedo = vec3(1.0);
	float roughness = 1.0;
	float metallic = 0.0;

	vec3 normal = in_normal;
	vec3 view = normalize(CAMERA_POSITION - in_position);
	vec3 light = normalize(vec3(1.0));

	vec3 radiance = vec3(1.0);
	vec3 out_light = light_calculate(normal, view, light, roughness, radiance, albedo, metallic);

	vec3 ambient = vec3(0.04);
	vec3 color = ambient + out_light;
	out_frag_color = vec4(color, 1.0);
}
