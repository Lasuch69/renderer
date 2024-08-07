#ifndef RENDERING_DEVICE_H
#define RENDERING_DEVICE_H

#include <rendering/resource_owner.h>
#include <rendering/typedefs.h>

#include "types/allocated.h"
#include "vulkan_context.h"

const uint32_t FRAMES_IN_FLIGHT = 2;

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

	VkCommandBuffer m_command_buffers[FRAMES_IN_FLIGHT];
	VkSemaphore m_present_semaphores[FRAMES_IN_FLIGHT];
	VkSemaphore m_render_semaphores[FRAMES_IN_FLIGHT];
	VkFence m_render_fences[FRAMES_IN_FLIGHT];

	VkDescriptorPool m_descriptor_pool;

	ResourceOwner<AllocatedBuffer> m_buffer_owner;

	VkCommandBuffer _begin_single_time_commands();
	void _end_single_time_commands(VkCommandBuffer command_buffer);

	AllocatedBuffer _buffer_create(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo *alloc_info = nullptr);
	void _buffer_copy(VkBuffer src_buffer, VkBuffer dst_buffer, size_t size);
	void _buffer_upload(VkBuffer buffer, const void *data, size_t size);
	void _buffer_destroy(AllocatedBuffer &buffer);

public:
	BufferID index_buffer_create(const void *data, size_t size);
	BufferID vertex_buffer_create(const void *data, size_t size);

	void buffer_copy(BufferID src_buffer, BufferID dst_buffer, size_t size);
	void buffer_upload(BufferID buffer, size_t offset, const void *data, size_t size);
	void buffer_destroy(BufferID buffer);

	void draw();

	void window_create(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void window_resize(uint32_t width, uint32_t height);

	void vulkan_init(const char *const *extensions, uint32_t extension_count, bool validation);
	void vulkan_free();

	inline VkInstance vulkan_instance() const {
		return m_context.instance();
	}
};

typedef RenderingDevice RD;

#endif // !RENDERING_DEVICE_H
