#ifndef RENDERER_H
#define RENDERER_H

#include <rendering/typedefs.h>

typedef struct VkInstance_T *VkInstance;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

typedef struct Mesh Mesh;

namespace math {
struct mat4;
} // namespace math

class Renderer {
public:
	static Renderer &singleton() {
		static Renderer instance;
		return instance;
	}

	Renderer(Renderer const &) = delete;
	void operator=(Renderer const &) = delete;

private:
	Renderer() {}

public:
	MeshID mesh_create(const Mesh &mesh);
	void mesh_destroy(MeshID mesh_id);

	MeshInstanceID mesh_instance_create();
	void mesh_instance_set_mesh(MeshInstanceID mesh_instance_id, MeshID mesh_id);
	void mesh_instance_set_transform(MeshInstanceID mesh_instance_id, const math::mat4 &transform);
	void mesh_instance_destroy(MeshInstanceID mesh_instance_id);

	void draw();

	void window_create(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void window_resize(uint32_t width, uint32_t height);

	void vulkan_create(const char **extensions, uint32_t extension_count, bool validation);
	void vulkan_destroy();

	VkInstance vulkan_instance() const;
};

#endif // !RENDERER_H
