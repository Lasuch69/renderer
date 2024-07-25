#ifndef TEXTURE_STORAGE_H
#define TEXTURE_STORAGE_H

#include <vulkan/vulkan_core.h>

#include "rendering/types/allocated.h"

typedef struct {
	AllocatedImage image;
	VkImageView imageView;
} Texture;

class TextureStorage {
public:
	static TextureStorage &singleton() {
		static TextureStorage instance;
		return instance;
	}

	TextureStorage(TextureStorage const &) = delete;
	void operator=(TextureStorage const &) = delete;

private:
	TextureStorage() {}

public:
};

#endif // !TEXTURE_STORAGE_H
