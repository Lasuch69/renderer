#include <cstdint>
#include <cstring>

#include <rendering/device/rendering_device.h>
#include <rendering/storage/mesh_storage.h>

#include "renderer.h"

MeshID Renderer::meshCreate(const Mesh &mesh) {
	return MeshStorage::singleton().meshCreate(mesh);
}

void Renderer::meshDestroy(MeshID meshID) {
	MeshStorage::singleton().meshDestroy(meshID);
}

MeshInstanceID Renderer::meshInstanceCreate() {
	return MeshStorage::singleton().meshInstanceCreate();
}

void Renderer::meshInstanceSetMesh(MeshInstanceID meshInstanceID, MeshID meshID) {
	MeshStorage::singleton().meshInstanceSetMesh(meshInstanceID, meshID);
}

void Renderer::meshInstanceSetTransform(MeshInstanceID meshInstanceID, const math::mat4 &transform) {
	MeshStorage::singleton().meshInstanceSetTransform(meshInstanceID, transform);
}

void Renderer::meshInstanceDestroy(MeshInstanceID meshInstanceID) {
	MeshStorage::singleton().meshInstanceDestroy(meshInstanceID);
}

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
