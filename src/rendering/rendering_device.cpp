#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "rendering_device.h"

#define CHECK_VK_RESULT(_expr, msg)                                                                                    \
	if (!(_expr)) {                                                                                                    \
		printf("%s\n", msg);                                                                                           \
	}

VkCommandBuffer RD::_beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = m_context.commandPool();
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_context.device(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void RD::_endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(m_context.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_context.graphicsQueue());
	vkFreeCommandBuffers(m_context.device(), m_context.commandPool(), 1, &commandBuffer);
}

AllocatedBuffer RD::bufferCreate(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo *allocInfo) {
	VkBufferCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	createInfo.size = size;
	createInfo.usage = usage;

	VmaAllocationCreateInfo allocCreateInfo = {};
	allocCreateInfo.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT;
	allocCreateInfo.usage = VMA_MEMORY_USAGE_AUTO;

	AllocatedBuffer buffer;
	vmaCreateBuffer(m_allocator, &createInfo, &allocCreateInfo, &buffer.handle, &buffer.allocation, allocInfo);
	return buffer;
}

void RD::bufferCopy(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size) {
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

	VkBufferCopy bufferCopy = {};
	bufferCopy.srcOffset = 0;
	bufferCopy.dstOffset = 0;
	bufferCopy.size = size;

	vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &bufferCopy);

	_endSingleTimeCommands(commandBuffer);
}

void RD::bufferUpdate(VkBuffer buffer, void *data, size_t size) {
	VmaAllocationInfo allocInfo;
	AllocatedBuffer staging = bufferCreate(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, &allocInfo);

	memcpy(allocInfo.pMappedData, data, size);
	vmaFlushAllocation(m_allocator, staging.allocation, 0, VK_WHOLE_SIZE);
	bufferCopy(staging.handle, buffer, size);
}

void RD::bufferDestroy(AllocatedBuffer buffer) {
	vmaDestroyBuffer(m_allocator, buffer.handle, buffer.allocation);
}

AllocatedImage RD::imageCreate(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage) {
	VkImageCreateInfo imageInfo = {};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = format;
	imageInfo.extent = { width, height, 1 };
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = usage;
	imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VmaAllocationCreateInfo allocInfo = {};
	allocInfo.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	allocInfo.usage = VMA_MEMORY_USAGE_AUTO;
	allocInfo.priority = 1.0f;

	AllocatedImage image;
	vmaCreateImage(m_allocator, &imageInfo, &allocInfo, &image.handle, &image.allocation, nullptr);

	return image;
}

void RD::imageUpdate(VkImage image, uint32_t width, uint32_t height, VkFormat format, void *data, size_t size) {
	VmaAllocationInfo stagingAllocInfo;
	VkBufferUsageFlags usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;

	AllocatedBuffer stagingBuffer = bufferCreate(size, usage, &stagingAllocInfo);
	memcpy(stagingAllocInfo.pMappedData, data, size);
	vmaFlushAllocation(m_allocator, stagingBuffer.allocation, 0, VK_WHOLE_SIZE);

	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.srcAccessMask = VK_ACCESS_NONE;
		imageBarrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.image = image;
		imageBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
				nullptr, 0, nullptr, 1, &imageBarrier);
	}

	{
		VkImageSubresourceLayers imageSubresource = {};
		imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		imageSubresource.mipLevel = 0;
		imageSubresource.baseArrayLayer = 0;
		imageSubresource.layerCount = 1;

		VkExtent3D imageExtent = {};
		imageExtent.width = width;
		imageExtent.height = height;
		imageExtent.depth = 1;

		VkBufferImageCopy region = {};
		region.imageSubresource = imageSubresource;
		region.imageExtent = imageExtent;

		vkCmdCopyBufferToImage(
				commandBuffer, stagingBuffer.handle, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	{
		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.baseMipLevel = 0;
		subresourceRange.levelCount = 1;
		subresourceRange.baseArrayLayer = 0;
		subresourceRange.layerCount = 1;

		VkImageMemoryBarrier imageBarrier = {};
		imageBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		imageBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		imageBarrier.image = image;
		imageBarrier.subresourceRange = subresourceRange;

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0,
				nullptr, 0, nullptr, 1, &imageBarrier);
	}

	_endSingleTimeCommands(commandBuffer);
	bufferDestroy(stagingBuffer);
}

void RD::imageDestroy(AllocatedImage image) {
	vmaDestroyImage(m_allocator, image.handle, image.allocation);
}

VkImageView RD::imageViewCreate(VkImage image, VkFormat format) {
	VkImageSubresourceRange subresourceRange = {};
	subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresourceRange.baseMipLevel = 0;
	subresourceRange.levelCount = 1;
	subresourceRange.baseArrayLayer = 0;
	subresourceRange.layerCount = 1;

	VkImageViewCreateInfo viewInfo = {};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = image;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = format;
	viewInfo.subresourceRange = subresourceRange;

	VkImageView view;
	CHECK_VK_RESULT(vkCreateImageView(m_context.device(), &viewInfo, nullptr, &view) == VK_SUCCESS,
			"Image view creation failed!");

	return view;
}

void RD::imageViewDestroy(VkImageView imageView) {
	vkDestroyImageView(m_context.device(), imageView, nullptr);
}

VkInstance RD::vulkanInstance() {
	return m_context.instance();
}

void RD::draw() {
	CHECK_VK_RESULT(vkWaitForFences(m_context.device(), 1, &m_renderFences[m_frame], VK_TRUE, UINT64_MAX) == VK_SUCCESS,
			"Fence timed out!");

	uint32_t imageIndex = 0;
	VkResult result = vkAcquireNextImageKHR(m_context.device(), m_context.swapchain(), UINT64_MAX,
			m_presentSemaphores[m_frame], VK_NULL_HANDLE, &imageIndex);

	if (result == VK_ERROR_OUT_OF_DATE_KHR) {
		m_context.windowResize(m_width, m_height);
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		printf("Swapchain image acquire failed!\n");
	}

	vkResetFences(m_context.device(), 1, &m_renderFences[m_frame]);

	VkCommandBuffer commandBuffer = m_commandBuffers[m_frame];
	vkResetCommandBuffer(commandBuffer, 0);

	VkCommandBufferBeginInfo beginInfo = {};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkClearValue clearValue = {};
	clearValue.color = { { 0.0f, 0.0f, 0.0f, 1.0f } };

	VkExtent2D extent = m_context.swapchainExtent();

	VkViewport viewport = {};
	viewport.width = (float)extent.width;
	viewport.height = (float)extent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	VkRect2D scissor = {};
	scissor.extent = extent;

	VkRect2D renderArea = {};
	renderArea.extent = extent;

	VkRenderPassBeginInfo renderPassInfo = {};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass = m_context.renderPass();
	renderPassInfo.framebuffer = m_context.framebuffer(imageIndex);
	renderPassInfo.renderArea = renderArea;
	renderPassInfo.clearValueCount = 1;
	renderPassInfo.pClearValues = &clearValue;

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdEndRenderPass(commandBuffer);
	vkEndCommandBuffer(commandBuffer);

	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = &m_presentSemaphores[m_frame];
	submitInfo.pWaitDstStageMask = &waitDstStageMask;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = &m_renderSemaphores[m_frame];

	vkQueueSubmit(m_context.graphicsQueue(), 1, &submitInfo, m_renderFences[m_frame]);

	VkSwapchainKHR swapchain = m_context.swapchain();

	VkPresentInfoKHR presentInfo = {};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = &m_renderSemaphores[m_frame];
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &swapchain;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(m_context.presentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_resized) {
		m_context.windowResize(m_width, m_height);
		m_resized = false;
	} else if (result != VK_SUCCESS) {
		printf("Swapchain image presentation failed!\n");
	}

	m_frame = (m_frame + 1) % FRAMES_IN_FLIGHT;
}

void RD::windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
	m_context.windowCreate(surface, width, height);

	m_width = width;
	m_height = height;

	// allocator

	{
		VmaAllocatorCreateInfo allocatorInfo = {};
		allocatorInfo.vulkanApiVersion = VK_API_VERSION_1_0;
		allocatorInfo.instance = m_context.instance();
		allocatorInfo.physicalDevice = m_context.physicalDevice();
		allocatorInfo.device = m_context.device();

		CHECK_VK_RESULT(vmaCreateAllocator(&allocatorInfo, &m_allocator) == VK_SUCCESS, "Allocator creation failed!");
	}

	// commands

	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_context.commandPool();
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = FRAMES_IN_FLIGHT;

		CHECK_VK_RESULT(vkAllocateCommandBuffers(m_context.device(), &allocInfo, m_commandBuffers) == VK_SUCCESS,
				"Command buffers allocation failed!");
	}

	// sync

	{
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			vkCreateSemaphore(m_context.device(), &semaphoreInfo, nullptr, &m_presentSemaphores[i]);
			vkCreateSemaphore(m_context.device(), &semaphoreInfo, nullptr, &m_renderSemaphores[i]);
			vkCreateFence(m_context.device(), &fenceInfo, nullptr, &m_renderFences[i]);
		}
	}

	// descriptor pool

	{
		VkDescriptorPoolSize poolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, FRAMES_IN_FLIGHT },
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1 },
		};

		uint32_t maxSets = 0;
		for (const VkDescriptorPoolSize &poolSize : poolSizes) {
			maxSets += poolSize.descriptorCount;
		}

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.maxSets = maxSets;
		poolInfo.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]);
		poolInfo.pPoolSizes = poolSizes;

		CHECK_VK_RESULT(vkCreateDescriptorPool(m_context.device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS,
				"Descriptor pool creation failed!");
	}

	m_initialized = true;
}

void RD::windowResize(uint32_t width, uint32_t height) {
	if (m_width == width && m_height == height)
		return;

	m_width = width;
	m_height = height;
	m_resized = true;
}

void RD::vulkanCreate(const char *const *extensions, uint32_t extensionCount, bool validation) {
	m_context.create(extensions, extensionCount, validation);
}

void RD::vulkanDestroy() {
	if (!m_context.isInitialized()) {
		m_context.destroy();
		return;
	}

	vkDeviceWaitIdle(m_context.device());

	if (m_initialized) {
		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			vkDestroySemaphore(m_context.device(), m_presentSemaphores[i], nullptr);
			vkDestroySemaphore(m_context.device(), m_renderSemaphores[i], nullptr);
			vkDestroyFence(m_context.device(), m_renderFences[i], nullptr);
		}

		vmaDestroyAllocator(m_allocator);
		m_initialized = false;
	}

	m_context.destroy();
}
