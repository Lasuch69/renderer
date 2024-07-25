#include <cstdint>
#include <cstring>

#include "rendering_device.h"
#include "rendering_server.h"

void RS::initialize(int argc, char **argv, const char **extensions, uint32_t extensionCount) {
	bool validation = false;
	for (int i = 0; i < argc; i++) {
		if (strcmp("--validate", argv[i]) == 0)
			validation = true;
	}

	RD::singleton().vulkanCreate(extensions, extensionCount, validation);
}

VkInstance RS::vulkanInstance() {
	return RD::singleton().vulkanInstance();
}

void RS::windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
	RD::singleton().windowCreate(surface, width, height);
}

void RS::windowResize(uint32_t width, uint32_t height) {
	RD::singleton().windowResize(width, height);
}

void RS::draw() {
	RD::singleton().draw();
}
