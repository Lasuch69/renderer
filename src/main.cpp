#include <cstdint>

#include "io/gltf_loader.h"
#include "rendering/rendering_device.h"

int main(int argc, char *argv[]) {
	GLTFLoader::loadFile(argv[1]);

	RD &rd = RD::singleton();

	const char *extensions[] = {
		VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
	};

	uint32_t extensionCount = sizeof(extensions) / sizeof(extensions[0]);
	rd.vulkanCreate(extensions, extensionCount, true);
	rd.vulkanDestroy();

	return 0;
}
