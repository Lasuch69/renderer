#include <cstdint>
#include <cstring>

#include <rendering/device/rendering_device.h>
#include <rendering/storage/mesh_storage.h>

#include "renderer.h"

MeshID Renderer::mesh_create(const Mesh &mesh) {
	return MeshStorage::singleton().mesh_create(mesh);
}

void Renderer::mesh_destroy(MeshID mesh_id) {
	MeshStorage::singleton().mesh_destroy(mesh_id);
}

MeshInstanceID Renderer::mesh_instance_create() {
	return MeshStorage::singleton().mesh_instance_create();
}

void Renderer::mesh_instance_set_mesh(MeshInstanceID mesh_instance_id, MeshID mesh_id) {
	MeshStorage::singleton().mesh_instance_set_mesh(mesh_instance_id, mesh_id);
}

void Renderer::mesh_instance_set_transform(MeshInstanceID mesh_instance_id, const math::mat4 &transform) {
	MeshStorage::singleton().mesh_instance_set_transform(mesh_instance_id, transform);
}

void Renderer::mesh_instance_destroy(MeshInstanceID mesh_instance_id) {
	MeshStorage::singleton().mesh_instance_destroy(mesh_instance_id);
}

void Renderer::draw() {
	RD::singleton().draw();
}

void Renderer::window_create(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
	RD::singleton().window_create(surface, width, height);
}

void Renderer::window_resize(uint32_t width, uint32_t height) {
	RD::singleton().window_resize(width, height);
}

void Renderer::vulkan_create(const char **extensions, uint32_t extension_count, bool validation) {
	RD::singleton().vulkan_init(extensions, extension_count, validation);
}

void Renderer::vulkan_destroy() {
	RD::singleton().vulkan_free();
}

VkInstance Renderer::vulkan_instance() const {
	return RD::singleton().vulkan_instance();
}
