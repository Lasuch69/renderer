#include <cstdint>
#include <cstdio>

#include <cgltf/cgltf.h>
#include <stb/stb_image.h>

#include "gltf_loader.h"

// PI * 4
const float CANDELA_TO_LUMEN = 12.566370614;

void GLTFLoader::loadFile(const char *path) {
	cgltf_options options = {};
	cgltf_data *data = NULL;

	if (cgltf_parse_file(&options, path, &data) != cgltf_result_success)
		return;

	for (uint64_t i = 0; i < data->lights_count; i++) {
		cgltf_light &light = data->lights[i];

		printf("%s\n", light.name);
		printf("%.3f lumens\n", light.intensity * CANDELA_TO_LUMEN);
	}

	cgltf_free(data);
}
