#ifndef RENDERING_DEVICE_H
#define RENDERING_DEVICE_H

#include <cstddef>
#include <cstdint>

#include "rendering/common/vk_allocated.h"

#include "vulkan_context.h"

const uint32_t FRAMES_IN_FLIGHT = 2;

typedef struct VmaAllocator_T *VmaAllocator;
typedef struct VmaAllocationInfo VmaAllocationInfo;

typedef struct {
	float PROJECTION_VIEW_MATRIX[16];
} CameraConstants;

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

	AllocatedBuffer m_instanceBuffer;
	uint32_t m_instanceCount;

	AllocatedBuffer m_testCubeIndexBuffer;
	AllocatedBuffer m_testCubeVertexBuffer;
	uint32_t m_testCubeIndexCount;

	VkDescriptorPool m_descriptorPool;

	VkDescriptorSetLayout m_gbufferSetLayout;
	VkDescriptorSet m_gbufferSet;

	VkPipelineLayout m_geometryPipelineLayout;
	VkPipeline m_geometryPipeline;

	VkPipelineLayout m_lightPipelineLayout;
	VkPipeline m_lightPipeline;

	void _updateGBufferInputAttachments();

	VkCommandBuffer _beginSingleTimeCommands();
	void _endSingleTimeCommands(VkCommandBuffer commandBuffer);

public:
	AllocatedBuffer bufferCreate(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo *allocInfo = nullptr);
	void bufferCopy(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size);
	void bufferUpdate(VkBuffer buffer, const void *data, size_t size);
	void bufferDestroy(AllocatedBuffer buffer);

	AllocatedImage imageCreate(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage);
	void imageUpdate(VkImage image, uint32_t width, uint32_t height, VkFormat format, void *data, size_t size);
	void imageDestroy(AllocatedImage image);

	VkImageView imageViewCreate(VkImage image, VkFormat format);
	void imageViewDestroy(VkImageView imageView);

	VkInstance vulkanInstance();

	void draw();

	void windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void windowResize(uint32_t width, uint32_t height);

	void vulkanCreate(const char *const *extensions, uint32_t extensionCount, bool validation);
	void vulkanDestroy();
};

typedef RenderingDevice RD;

#endif // !RENDERING_DEVICE_H
