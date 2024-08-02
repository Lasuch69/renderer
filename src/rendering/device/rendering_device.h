#ifndef RENDERING_DEVICE_H
#define RENDERING_DEVICE_H

#include <cstddef>
#include <cstdint>

#include "types/allocated.h"
#include "types/resource_owner.h"

#include "vulkan_context.h"

const uint32_t FRAMES_IN_FLIGHT = 2;

typedef size_t BufferID;

typedef struct VmaAllocator_T *VmaAllocator;
typedef struct VmaAllocationInfo VmaAllocationInfo;

class RenderingDevice {
public:
	static RenderingDevice &singleton() {
		static RenderingDevice instance;
		return instance;
	}

	RenderingDevice(RenderingDevice const &) = delete;
	void operator=(RenderingDevice const &) = delete;

private:
	RenderingDevice() {}

	VulkanContext m_context;
	bool m_initialized = false;

	uint32_t m_frame = 0;
	uint32_t m_width, m_height;
	bool m_resized = false;

	VmaAllocator m_allocator;

	VkCommandBuffer m_commandBuffers[FRAMES_IN_FLIGHT];
	VkSemaphore m_presentSemaphores[FRAMES_IN_FLIGHT];
	VkSemaphore m_renderSemaphores[FRAMES_IN_FLIGHT];
	VkFence m_renderFences[FRAMES_IN_FLIGHT];

	VkDescriptorPool m_descriptorPool;
	VkDescriptorSetLayout m_uniformSetLayout;

	ResourceOwner<AllocatedBuffer> m_bufferOwner;

	VkCommandBuffer _beginSingleTimeCommands();
	void _endSingleTimeCommands(VkCommandBuffer commandBuffer);

	AllocatedBuffer _bufferCreate(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo *allocInfo = nullptr);
	void _bufferCopy(VkBuffer src, VkBuffer dst, size_t size);
	void _bufferUpload(VkBuffer buffer, const void *data, size_t size);
	void _bufferDestroy(AllocatedBuffer &buffer);

public:
	BufferID bufferCreate(const void *data, size_t size);
	void bufferCopy(BufferID src, BufferID dst, size_t size);
	void bufferUpload(BufferID buffer, size_t offset, const void *data, size_t size);
	void bufferDestroy(BufferID buffer);

	void draw();

	void windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void windowResize(uint32_t width, uint32_t height);

	void vkCreate(const char *const *extensions, uint32_t extensionCount, bool validation);
	void vkDestroy();

	inline VkInstance vkInstance() const {
		return m_context.instance();
	}
};

typedef RenderingDevice RD;

#endif // !RENDERING_DEVICE_H
