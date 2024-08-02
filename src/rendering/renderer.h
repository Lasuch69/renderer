#ifndef RENDERER_H
#define RENDERER_H

#include <cstdint>

typedef struct VkInstance_T *VkInstance;
typedef struct VkSurfaceKHR_T *VkSurfaceKHR;

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
	void draw();

	void windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void windowResize(uint32_t width, uint32_t height);

	void vkCreate(const char **extensions, uint32_t extensionCount, bool validation);
	void vkDestroy();

	VkInstance vkInstance() const;
};

#endif // !RENDERER_H
