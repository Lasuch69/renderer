#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include <rendering/resource_owner.h>
#include <rendering/typedefs.h>

#include "rendering_device.h"

#define CHECK_VK_RESULT(_expr, msg)                                                                                    \
	if (!(_expr)) {                                                                                                    \
		printf("%s\n", msg);                                                                                           \
	}

VkCommandBuffer RD::_begin_single_time_commands() {
	VkCommandBufferAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	alloc_info.commandPool = m_context.command_pool();
	alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	alloc_info.commandBufferCount = 1;

	VkCommandBuffer command_buffer;
	vkAllocateCommandBuffers(m_context.device(), &alloc_info, &command_buffer);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(command_buffer, &begin_info);
	return command_buffer;
}

void RD::_end_single_time_commands(VkCommandBuffer command_buffer) {
	vkEndCommandBuffer(command_buffer);

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	vkQueueSubmit(m_context.graphics_queue(), 1, &submit_info, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_context.graphics_queue());
	vkFreeCommandBuffers(m_context.device(), m_context.command_pool(), 1, &command_buffer);
}

AllocatedBuffer RD::_buffer_create(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo *alloc_info) {
	VkBufferCreateInfo buffer_Info = {};
	buffer_Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_Info.size = size;
	buffer_Info.usage = usage;

	VmaAllocationCreateInfo allocation_info = {};
	allocation_info.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
	allocation_info.usage = VMA_MEMORY_USAGE_AUTO;

	VkBuffer buffer;
	VmaAllocation allocation;
	vmaCreateBuffer(m_allocator, &buffer_Info, &allocation_info, &buffer, &allocation, alloc_info);

	return { Allocation{ allocation, size }, buffer };
}

void RD::_buffer_copy(VkBuffer src_buffer, VkBuffer dst_buffer, size_t size) {
	VkCommandBuffer command_buffer = _begin_single_time_commands();

	VkBufferCopy buffer_copy = {};
	buffer_copy.srcOffset = 0;
	buffer_copy.dstOffset = 0;
	buffer_copy.size = size;

	vkCmdCopyBuffer(command_buffer, src_buffer, dst_buffer, 1, &buffer_copy);

	_end_single_time_commands(command_buffer);
}

void RD::_buffer_upload(VkBuffer buffer, const void *data, size_t size) {
	VmaAllocationInfo alloc_info;
	AllocatedBuffer staging = _buffer_create(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &alloc_info);

	memcpy(alloc_info.pMappedData, data, size);
	vmaFlushAllocation(m_allocator, staging.allocation.handle, 0, VK_WHOLE_SIZE);

	_buffer_copy(staging.buffer, buffer, size);
	_buffer_destroy(staging);
}

void RD::_buffer_destroy(AllocatedBuffer &buffer) {
	vmaDestroyBuffer(m_allocator, buffer.buffer, buffer.allocation.handle);
	buffer.allocation.size = 0;
}

BufferID RD::index_buffer_create(const void *data, size_t size) {
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	AllocatedBuffer allocated_buffer = _buffer_create(size, usage);

	if (data != nullptr)
		_buffer_upload(allocated_buffer.buffer, data, size);

	return m_buffer_owner.insert(allocated_buffer);
}

BufferID RD::vertex_buffer_create(const void *data, size_t size) {
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
	AllocatedBuffer allocated_buffer = _buffer_create(size, usage);

	if (data != nullptr)
		_buffer_upload(allocated_buffer.buffer, data, size);

	return m_buffer_owner.insert(allocated_buffer);
}

void RD::buffer_copy(BufferID src_buffer, BufferID dst_buffer, size_t size) {
	const AllocatedBuffer *_src_buffer = m_buffer_owner.get(src_buffer);
	const AllocatedBuffer *_dst_buffer = m_buffer_owner.get(dst_buffer);

	if (_src_buffer == nullptr || _dst_buffer == nullptr)
		return;

	_buffer_copy(_src_buffer->buffer, _dst_buffer->buffer, size);
}

void RD::buffer_upload(BufferID buffer, size_t offset, const void *data, size_t size) {
	const AllocatedBuffer *_buffer = m_buffer_owner.get(buffer);

	if (_buffer == nullptr)
		return;

	_buffer_upload(_buffer->buffer, data, size);
}

void RD::buffer_destroy(BufferID buffer) {
	AllocatedBuffer *_buffer = m_buffer_owner.get(buffer);

	if (_buffer == nullptr)
		return;

	_buffer_destroy(*_buffer);
	m_buffer_owner.remove(buffer);
}

void RD::cmd_bind_index_buffer(BufferID index_buffer) {
	assert(m_render_handle.is_active);
	assert(m_buffer_owner.has(index_buffer));

	VkCommandBuffer command_buffer = m_render_handle.command_buffer;
	vkCmdBindIndexBuffer(command_buffer, m_buffer_owner.get(index_buffer)->buffer, 0, VK_INDEX_TYPE_UINT32);
}

void RD::cmd_bind_vertex_buffer(BufferID vertex_buffer) {
	assert(m_render_handle.is_active);
	assert(m_buffer_owner.has(vertex_buffer));

	VkDeviceSize offset = 0;
	VkCommandBuffer command_buffer = m_render_handle.command_buffer;
	vkCmdBindVertexBuffers(command_buffer, 0, 1, &m_buffer_owner.get(vertex_buffer)->buffer, &offset);
}
void RD::cmd_draw_indexed(uint32_t index_count, uint32_t instance_count, uint32_t first_index, int32_t vertex_offset,
		uint32_t first_instance) {
	assert(m_render_handle.is_active);

	VkCommandBuffer command_buffer = m_render_handle.command_buffer;
	vkCmdDrawIndexed(command_buffer, index_count, instance_count, first_index, vertex_offset, first_instance);
}

void RD::draw_begin() {
	CHECK_VK_RESULT(
			vkWaitForFences(m_context.device(), 1, &m_render_fences[m_frame], VK_TRUE, UINT64_MAX) == VK_SUCCESS,
			"Fence timed out!");

	uint32_t image_index = 0;
	VkResult result = vkAcquireNextImageKHR(m_context.device(), m_context.swapchain(), UINT64_MAX,
			m_present_semaphores[m_frame], VK_NULL_HANDLE, &image_index);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		m_context.window_resize(m_width, m_height);
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		printf("Swapchain image acquire failed!\n");
	}

	vkResetFences(m_context.device(), 1, &m_render_fences[m_frame]);

	VkCommandBuffer command_buffer = m_command_buffers[m_frame];
	vkResetCommandBuffer(command_buffer, 0);

	VkCommandBufferBeginInfo begin_info = {};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(command_buffer, &begin_info);

	VkClearValue clear_value = {};
	clear_value.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkExtent2D extent = m_context.swapchain_extent();

	VkViewport viewport = {};
	viewport.width = static_cast<float>(extent.width);
	viewport.height = static_cast<float>(extent.height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.extent = extent;

	VkRect2D render_area = {};
	render_area.extent = extent;

	VkRenderPassBeginInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	render_pass_info.renderPass = m_context.render_pass();
	render_pass_info.framebuffer = m_context.framebuffer(image_index);
	render_pass_info.renderArea = render_area;
	render_pass_info.clearValueCount = 1;
	render_pass_info.pClearValues = &clear_value;

	vkCmdBeginRenderPass(command_buffer, &render_pass_info, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdSetViewport(command_buffer, 0, 1, &viewport);
	vkCmdSetScissor(command_buffer, 0, 1, &scissor);

	m_render_handle.command_buffer = command_buffer;
	m_render_handle.image_index = image_index;
	m_render_handle.is_active = true;
}

void RD::draw_submit() {
	assert(m_render_handle.is_active);

	VkCommandBuffer command_buffer = m_render_handle.command_buffer;
	uint32_t image_index = m_render_handle.image_index;
	m_render_handle.is_active = false;

	vkCmdEndRenderPass(command_buffer);
	vkEndCommandBuffer(command_buffer);

	VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submit_info = {};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = &m_present_semaphores[m_frame];
	submit_info.pWaitDstStageMask = &wait_dst_stage_mask;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = &m_render_semaphores[m_frame];

	vkQueueSubmit(m_context.graphics_queue(), 1, &submit_info, m_render_fences[m_frame]);

	VkSwapchainKHR swapchain = m_context.swapchain();

	VkPresentInfoKHR present_info = {};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = &m_render_semaphores[m_frame];
	present_info.swapchainCount = 1;
	present_info.pSwapchains = &swapchain;
	present_info.pImageIndices = &image_index;

	VkResult result = vkQueuePresentKHR(m_context.present_queue(), &present_info);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_resized) {
		m_context.window_resize(m_width, m_height);
		m_resized = false;
	} else if (result != VK_SUCCESS) {
		printf("Swapchain image presentation failed!\n");
	}

	m_frame = (m_frame + 1) % FRAMES_IN_FLIGHT;
}

void RD::window_create(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
	m_context.window_create(surface, width, height);

	m_width = width;
	m_height = height;

	// allocator

	{
		VmaAllocatorCreateInfo allocator_info = {};
		allocator_info.vulkanApiVersion = VK_API_VERSION_1_0;
		allocator_info.instance = m_context.instance();
		allocator_info.physicalDevice = m_context.physical_device();
		allocator_info.device = m_context.device();

		CHECK_VK_RESULT(vmaCreateAllocator(&allocator_info, &m_allocator) == VK_SUCCESS, "Allocator creation failed!");
	}

	// commands

	{
		VkCommandBufferAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		alloc_info.commandPool = m_context.command_pool();
		alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		alloc_info.commandBufferCount = FRAMES_IN_FLIGHT;

		CHECK_VK_RESULT(vkAllocateCommandBuffers(m_context.device(), &alloc_info, m_command_buffers) == VK_SUCCESS,
				"Command buffers allocation failed!");
	}

	// sync

	{
		VkSemaphoreCreateInfo semaphore_info = {};
		semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fence_info = {};
		fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			vkCreateSemaphore(m_context.device(), &semaphore_info, nullptr, &m_present_semaphores[i]);
			vkCreateSemaphore(m_context.device(), &semaphore_info, nullptr, &m_render_semaphores[i]);
			vkCreateFence(m_context.device(), &fence_info, nullptr, &m_render_fences[i]);
		}
	}

	// descriptor pool

	{
		VkDescriptorPoolSize pool_sizes[] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 },
		};

		uint32_t max_sets = 0;
		for (const VkDescriptorPoolSize &pool_size : pool_sizes) {
			max_sets += pool_size.descriptorCount;
		}

		VkDescriptorPoolCreateInfo pool_info = {};
		pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		pool_info.maxSets = max_sets;
		pool_info.poolSizeCount = sizeof(pool_sizes) / sizeof(pool_sizes[0]);
		pool_info.pPoolSizes = pool_sizes;

		CHECK_VK_RESULT(
				vkCreateDescriptorPool(m_context.device(), &pool_info, nullptr, &m_descriptor_pool) == VK_SUCCESS,
				"Descriptor pool creation failed!");
	}

	m_initialized = true;
}

void RD::window_resize(uint32_t width, uint32_t height) {
	if (m_width == width && m_height == height)
		return;

	m_width = width;
	m_height = height;
	m_resized = true;
}

void RD::vulkan_init(const char *const *extensions, uint32_t extension_count, bool validation) {
	m_context.create(extensions, extension_count, validation);
}

void RD::vulkan_free() {
	if (!m_context.initialized()) {
		m_context.destroy();
		return;
	}

	vkDeviceWaitIdle(m_context.device());

	if (m_initialized) {
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_context.device(), m_present_semaphores[i], nullptr);
			vkDestroySemaphore(m_context.device(), m_render_semaphores[i], nullptr);
			vkDestroyFence(m_context.device(), m_render_fences[i], nullptr);
		}

		vmaDestroyAllocator(m_allocator);
		m_initialized = false;
	}

	m_context.destroy();
}
