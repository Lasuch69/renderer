#include <cstdint>
#include <cstdio>
#include <cstring>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "rendering/rendering_server.h"

const int WIDTH = 800;
const int HEIGHT = 600;

int main(int argc, char *argv[]) {
	bool useWayland = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--wayland") == 0)
			useWayland = true;
	}

	if (useWayland) {
		SDL_SetHint(SDL_HINT_VIDEODRIVER, "wayland");
	} else {
		SDL_SetHint(SDL_HINT_VIDEODRIVER, "x11");
	}

	if (SDL_Init(SDL_INIT_VIDEO)) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
		return EXIT_FAILURE;
	}

	uint32_t flags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_VULKAN;
	SDL_Window *window = SDL_CreateWindow("App", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, flags);

	if (window == nullptr) {
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "%s", SDL_GetError());
		SDL_Quit();

		return EXIT_FAILURE;
	}

	uint32_t extensionCount = 0;
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, nullptr);

	const char **extensions = (const char **)malloc(extensionCount * sizeof(const char *));
	SDL_Vulkan_GetInstanceExtensions(window, &extensionCount, extensions);

	RS::singleton().initialize(argc, argv, extensions, extensionCount);
	VkInstance instance = RS::singleton().vulkanInstance();

	VkSurfaceKHR surface;
	SDL_Vulkan_CreateSurface(window, instance, &surface);
	RS::singleton().windowCreate(surface, WIDTH, HEIGHT);

	bool quit = false;
	while (!quit) {
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT)
				quit = true;

			if (event.type == SDL_WINDOWEVENT_RESIZED) {
				int width, height;
				SDL_Vulkan_GetDrawableSize(window, &width, &height);
				RS::singleton().windowResize(width, height);
			}
		}

		RS::singleton().draw();
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return EXIT_SUCCESS;
}
