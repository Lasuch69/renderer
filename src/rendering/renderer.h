#ifndef RENDERER_H
#define RENDERER_H

#include <cstddef>
#include <cstdint>

typedef struct VkInstance_T *VkInstance;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

typedef struct Mesh Mesh;

typedef size_t MeshID;
typedef size_t MeshInstanceID;

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
	MeshID meshCreate(const Mesh &mesh);
	void meshDestroy(MeshID meshID);

	MeshInstanceID meshInstanceCreate();
	void meshInstanceSetMesh(MeshInstanceID meshInstanceID, MeshID meshID);
	void meshInstanceSetTransform(MeshInstanceID meshInstanceID, const math::mat4 &transform);
	void meshInstanceDestroy(MeshInstanceID meshInstanceID);

	void draw();

	void windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void windowResize(uint32_t width, uint32_t height);

	void vkCreate(const char **extensions, uint32_t extensionCount, bool validation);
	void vkDestroy();

	VkInstance vkInstance() const;
};

#endif // !RENDERER_H
