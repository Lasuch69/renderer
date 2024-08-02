#include <cstdint>
#include <cstring>

#include <rendering/device/rendering_device.h>

#include "renderer.h"

void Renderer::draw() {
	RD::singleton().draw();
}

void Renderer::windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
	RD::singleton().windowCreate(surface, width, height);
}

void Renderer::windowResize(uint32_t width, uint32_t height) {
	RD::singleton().windowResize(width, height);
}

void Renderer::vkCreate(const char **extensions, uint32_t extensionCount, bool validation) {
	RD::singleton().vkCreate(extensions, extensionCount, validation);
}

void Renderer::vkDestroy() {
	RD::singleton().vkDestroy();
}

VkInstance Renderer::vkInstance() const {
	return RD::singleton().vkInstance();
}
