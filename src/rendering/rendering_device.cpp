#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vma/vk_mem_alloc.h>
#include <vulkan/vulkan_core.h>

#include "shaders/light_pass.gen.h"

#include "rendering_device.h"

#define CHECK_VK_RESULT(_expr, msg)                                                                                    \
	if (!(_expr)) {                                                                                                    \
		printf("%s\n", msg);                                                                                           \
	}

VkResult pipelineCreate(VkDevice device, VkShaderModule vertexModule, VkShaderModule fragmentModule,
		VkPipelineLayout pipelineLayout, VkRenderPass renderPass, uint32_t subpass, VkPipeline *pipeline) {
	VkPipelineShaderStageCreateInfo vertexStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_VERTEX_BIT,
		.module = vertexModule,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo fragmentStageInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		.stage = VK_SHADER_STAGE_FRAGMENT_BIT,
		.module = fragmentModule,
		.pName = "main",
	};

	VkPipelineShaderStageCreateInfo shaderStageInfos[2] = {
		vertexStageInfo,
		fragmentStageInfo,
	};

	VkPipelineVertexInputStateCreateInfo vertexInputStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
	};

	VkPipelineViewportStateCreateInfo viewportStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		.viewportCount = 1,
		.scissorCount = 1,
	};

	VkPipelineRasterizationStateCreateInfo rasterizationStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		.rasterizerDiscardEnable = VK_FALSE,
		.polygonMode = VK_POLYGON_MODE_FILL,
		.cullMode = VK_CULL_MODE_BACK_BIT,
		.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
		.depthBiasEnable = VK_FALSE,
		.lineWidth = 1.0f,
	};

	VkPipelineMultisampleStateCreateInfo multisampleStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
		.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
		.sampleShadingEnable = VK_FALSE,
	};

	VkPipelineDepthStencilStateCreateInfo depthStencilStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		.depthTestEnable = VK_FALSE,
		.depthWriteEnable = VK_FALSE,
		.depthCompareOp = VK_COMPARE_OP_NEVER,
		.depthBoundsTestEnable = VK_FALSE,
		.stencilTestEnable = VK_FALSE,
	};

	VkPipelineColorBlendAttachmentState colorBlendAttachment = {
		.blendEnable = VK_FALSE,
		.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
						  VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlendStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		.logicOpEnable = VK_FALSE,
		.logicOp = VK_LOGIC_OP_COPY,
		.attachmentCount = 1,
		.pAttachments = &colorBlendAttachment,
		.blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
	};

	VkDynamicState dynamicStates[2] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};

	VkPipelineDynamicStateCreateInfo dynamicStateInfo = {
		.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		.dynamicStateCount = 2,
		.pDynamicStates = dynamicStates,
	};

	VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		.stageCount = 2,
		.pStages = shaderStageInfos,
		.pVertexInputState = &vertexInputStateInfo,
		.pInputAssemblyState = &inputAssemblyStateInfo,
		.pViewportState = &viewportStateInfo,
		.pRasterizationState = &rasterizationStateInfo,
		.pMultisampleState = &multisampleStateInfo,
		.pDepthStencilState = &depthStencilStateInfo,
		.pColorBlendState = &colorBlendStateInfo,
		.pDynamicState = &dynamicStateInfo,
		.layout = pipelineLayout,
		.renderPass = renderPass,
		.subpass = subpass,
	};

	return vkCreateGraphicsPipelines(device, nullptr, 1, &pipelineCreateInfo, nullptr, pipeline);
}

void RD::_updateGBufferInputAttachments() {
	VkDescriptorImageInfo albedoInfo = {
		.imageView = m_context.albedoAttachment().imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkDescriptorImageInfo normalInfo = {
		.imageView = m_context.normalAttachment().imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkDescriptorImageInfo roughnessMetallicInfo = {
		.imageView = m_context.roughnessMetallicAttachment().imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkDescriptorImageInfo depthInfo = {
		.imageView = m_context.depthAttachment().imageView,
		.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkWriteDescriptorSet albedoWriteInfo = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_gbufferSet,
		.dstBinding = 0,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		.pImageInfo = &albedoInfo,
	};

	VkWriteDescriptorSet normalWriteInfo = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_gbufferSet,
		.dstBinding = 1,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		.pImageInfo = &normalInfo,
	};

	VkWriteDescriptorSet roughnessMetallicWriteInfo = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_gbufferSet,
		.dstBinding = 2,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		.pImageInfo = &roughnessMetallicInfo,
	};

	VkWriteDescriptorSet depthWriteInfo = {
		.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
		.dstSet = m_gbufferSet,
		.dstBinding = 3,
		.descriptorCount = 1,
		.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
		.pImageInfo = &depthInfo,
	};

	VkWriteDescriptorSet writeInfos[] = {
		albedoWriteInfo,
		normalWriteInfo,
		roughnessMetallicWriteInfo,
		depthWriteInfo,
	};

	uint32_t writeInfoCount = sizeof(writeInfos) / sizeof(writeInfos[0]);

	vkUpdateDescriptorSets(m_context.device(), writeInfoCount, writeInfos, 0, nullptr);
}

VkCommandBuffer RD::_beginSingleTimeCommands() {
	VkCommandBufferAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool = m_context.commandPool(),
		.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount = 1,
	};

	VkCommandBuffer commandBuffer;
	vkAllocateCommandBuffers(m_context.device(), &allocInfo, &commandBuffer);

	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(commandBuffer, &beginInfo);
	return commandBuffer;
}

void RD::_endSingleTimeCommands(VkCommandBuffer commandBuffer) {
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
	};

	vkQueueSubmit(m_context.graphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(m_context.graphicsQueue());
	vkFreeCommandBuffers(m_context.device(), m_context.commandPool(), 1, &commandBuffer);
}

AllocatedBuffer RD::bufferCreate(size_t size, VkBufferUsageFlags usage, VmaAllocationInfo *allocInfo) {
	VkBufferCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size = size,
		.usage = usage,
	};

	VmaAllocationCreateInfo allocCreateInfo = {
		.flags = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
	};

	AllocatedBuffer buffer;
	vmaCreateBuffer(m_allocator, &createInfo, &allocCreateInfo, &buffer.handle, &buffer.allocation, allocInfo);
	return buffer;
}

void RD::bufferCopy(VkBuffer srcBuffer, VkBuffer dstBuffer, size_t size) {
	VkCommandBuffer commandBuffer = _beginSingleTimeCommands();

	VkBufferCopy bufferCopy = {
		.srcOffset = 0,
		.dstOffset = 0,
		.size = size,
	};

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
	VkImageCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		.imageType = VK_IMAGE_TYPE_2D,
		.format = format,
		.extent = { width, height, 1 },
		.mipLevels = 1,
		.arrayLayers = 1,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.tiling = VK_IMAGE_TILING_OPTIMAL,
		.usage = usage,
		.sharingMode = VK_SHARING_MODE_EXCLUSIVE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VmaAllocationCreateInfo allocInfo = {
		.flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
		.usage = VMA_MEMORY_USAGE_AUTO,
		.priority = 1.0f,
	};

	AllocatedImage image;
	vmaCreateImage(m_allocator, &createInfo, &allocInfo, &image.handle, &image.allocation, nullptr);

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
		VkImageSubresourceRange subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		VkImageMemoryBarrier imageBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_NONE,
			.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
			.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = subresourceRange,
		};

		vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0,
				nullptr, 0, nullptr, 1, &imageBarrier);
	}

	{
		VkImageSubresourceLayers imageSubresource = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.mipLevel = 0,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		VkExtent3D imageExtent = {
			.width = width,
			.height = height,
			.depth = 1,
		};

		VkBufferImageCopy region = {
			.imageSubresource = imageSubresource,
			.imageExtent = imageExtent,
		};

		vkCmdCopyBufferToImage(
				commandBuffer, stagingBuffer.handle, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);
	}

	{
		VkImageSubresourceRange subresourceRange = {
			.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel = 0,
			.levelCount = 1,
			.baseArrayLayer = 0,
			.layerCount = 1,
		};

		VkImageMemoryBarrier imageBarrier = {
			.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
			.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
			.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
			.image = image,
			.subresourceRange = subresourceRange,
		};

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
	VkImageSubresourceRange subresourceRange = {
		.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	VkImageViewCreateInfo viewInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = subresourceRange,
	};

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
		_updateGBufferInputAttachments();
	} else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
		printf("Swapchain image acquire failed!\n");
	}

	vkResetFences(m_context.device(), 1, &m_renderFences[m_frame]);

	VkCommandBuffer commandBuffer = m_commandBuffers[m_frame];
	vkResetCommandBuffer(commandBuffer, 0);

	VkCommandBufferBeginInfo beginInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
	};

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkExtent2D extent = m_context.swapchainExtent();

	VkViewport viewport = {
		.width = (float)extent.width,
		.height = (float)extent.height,
		.minDepth = 0.0f,
		.maxDepth = 1.0f,
	};

	VkRect2D scissor = {
		.extent = extent,
	};

	VkRect2D renderArea = {
		.extent = extent,
	};

	VkClearValue clearValues[5];
	clearValues[1].color = VkClearColorValue{ { 0.0, 0.0, 0.0, 0.0 } };
	clearValues[2].color = VkClearColorValue{ { 0.0, 0.0, 0.0, 0.0 } };
	clearValues[3].color = VkClearColorValue{ { 0.0, 0.0, 0.0, 0.0 } };
	clearValues[4].depthStencil = VkClearDepthStencilValue{ 1.0, 0 };

	uint32_t clearValueCount = sizeof(clearValues) / sizeof(clearValues[0]);

	VkRenderPassBeginInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		.renderPass = m_context.renderPass(),
		.framebuffer = m_context.framebuffer(imageIndex),
		.renderArea = renderArea,
		.clearValueCount = clearValueCount,
		.pClearValues = clearValues,
	};

	vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
	vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

	vkCmdNextSubpass(commandBuffer, VK_SUBPASS_CONTENTS_INLINE);

	vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_lightPipeline);
	vkCmdBindDescriptorSets(
			commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_lightPipelineLayout, 0, 1, &m_gbufferSet, 0, nullptr);

	vkCmdDraw(commandBuffer, 3, 1, 0, 0);

	vkCmdEndRenderPass(commandBuffer);
	vkEndCommandBuffer(commandBuffer);

	VkPipelineStageFlags waitDstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

	VkSubmitInfo submitInfo = {
		.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_presentSemaphores[m_frame],
		.pWaitDstStageMask = &waitDstStageMask,
		.commandBufferCount = 1,
		.pCommandBuffers = &commandBuffer,
		.signalSemaphoreCount = 1,
		.pSignalSemaphores = &m_renderSemaphores[m_frame],
	};

	vkQueueSubmit(m_context.graphicsQueue(), 1, &submitInfo, m_renderFences[m_frame]);

	VkSwapchainKHR swapchain = m_context.swapchain();

	VkPresentInfoKHR presentInfo = {
		.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
		.waitSemaphoreCount = 1,
		.pWaitSemaphores = &m_renderSemaphores[m_frame],
		.swapchainCount = 1,
		.pSwapchains = &swapchain,
		.pImageIndices = &imageIndex,
	};

	result = vkQueuePresentKHR(m_context.presentQueue(), &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_resized) {
		m_context.windowResize(m_width, m_height);
		_updateGBufferInputAttachments();

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
		VkCommandBufferAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			.commandPool = m_context.commandPool(),
			.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			.commandBufferCount = FRAMES_IN_FLIGHT,
		};

		CHECK_VK_RESULT(vkAllocateCommandBuffers(m_context.device(), &allocInfo, m_commandBuffers) == VK_SUCCESS,
				"Command buffers allocation failed!");
	}

	// sync

	{
		VkSemaphoreCreateInfo semaphoreInfo = {
			.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		};

		VkFenceCreateInfo fenceInfo = {
			.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VK_FENCE_CREATE_SIGNALED_BIT,
		};

		for (uint32_t i = 0; i < FRAMES_IN_FLIGHT; i++) {
			vkCreateSemaphore(m_context.device(), &semaphoreInfo, nullptr, &m_presentSemaphores[i]);
			vkCreateSemaphore(m_context.device(), &semaphoreInfo, nullptr, &m_renderSemaphores[i]);
			vkCreateFence(m_context.device(), &fenceInfo, nullptr, &m_renderFences[i]);
		}
	}

	// descriptor pool

	{
		VkDescriptorPoolSize poolSizes[] = {
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 4 },
		};

		uint32_t maxSets = 0;
		for (const VkDescriptorPoolSize &poolSize : poolSizes) {
			maxSets += poolSize.descriptorCount;
		}

		VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			.maxSets = maxSets,
			.poolSizeCount = sizeof(poolSizes) / sizeof(poolSizes[0]),
			.pPoolSizes = poolSizes,
		};

		CHECK_VK_RESULT(vkCreateDescriptorPool(m_context.device(), &descriptorPoolCreateInfo, nullptr,
								&m_descriptorPool) == VK_SUCCESS,
				"Descriptor pool creation failed!");
	}

	// gbuffer

	{
		VkDescriptorSetLayoutBinding albedoBinding = {
			.binding = 0,
			.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		VkDescriptorSetLayoutBinding normalBinding = {
			.binding = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		VkDescriptorSetLayoutBinding roughnessMetallicBinding = {
			.binding = 2,
			.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		VkDescriptorSetLayoutBinding depthBinding = {
			.binding = 3,
			.descriptorType = VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT,
			.descriptorCount = 1,
			.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
		};

		VkDescriptorSetLayoutBinding bindings[] = {
			albedoBinding,
			normalBinding,
			roughnessMetallicBinding,
			depthBinding,
		};

		uint32_t bindingCount = sizeof(bindings) / sizeof(bindings[0]);

		VkDescriptorSetLayoutCreateInfo createInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
			.bindingCount = bindingCount,
			.pBindings = bindings,
		};

		CHECK_VK_RESULT(vkCreateDescriptorSetLayout(m_context.device(), &createInfo, nullptr, &m_gbufferSetLayout) ==
								VK_SUCCESS,
				"GBuffer descriptor set layout creation failed!");

		VkDescriptorSetAllocateInfo allocInfo = {
			.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
			.descriptorPool = m_descriptorPool,
			.descriptorSetCount = 1,
			.pSetLayouts = &m_gbufferSetLayout,
		};

		CHECK_VK_RESULT(vkAllocateDescriptorSets(m_context.device(), &allocInfo, &m_gbufferSet) == VK_SUCCESS,
				"GBuffer descriptor set allocation failed!");

		_updateGBufferInputAttachments();
	}

	{
		VkPipelineLayoutCreateInfo layoutInfo = {
			.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
			.setLayoutCount = 1,
			.pSetLayouts = &m_gbufferSetLayout,
		};

		CHECK_VK_RESULT(
				vkCreatePipelineLayout(m_context.device(), &layoutInfo, nullptr, &m_lightPipelineLayout) == VK_SUCCESS,
				"Light pipeline layout creation failed!");

		LightPassShader shader;
		shader.compile(m_context.device());

		CHECK_VK_RESULT(
				pipelineCreate(m_context.device(), shader.vertexStage(), shader.fragmentStage(), m_lightPipelineLayout,
						m_context.renderPass(), LIGHT_PASS, &m_lightPipeline) == VK_SUCCESS,
				"Light pipeline creation failed!");
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
