#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <stdexcept>
#include <vulkan/vulkan_core.h>

#include "vulkan_context.h"

#define CHECK_VK_RESULT(_expr, msg)                                                                                    \
	if (!(_expr)) {                                                                                                    \
		printf("%s\n", msg);                                                                                           \
	}

const char *VALIDATION_LAYERS[1] = {
	"VK_LAYER_KHRONOS_validation",
};

const char *DEVICE_EXTENSIONS[1] = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

static uint32_t vk_clamp(uint32_t value, uint32_t min, uint32_t max) {
	uint32_t _max = value < max ? value : max;
	return min > _max ? min : _max;
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
		VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT *callback_data,
		void *user_data) {
	if (message_severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		return VK_FALSE;
	}

	printf("%s\n", callback_data->pMessage);
	return VK_FALSE;
}

VkResult create_debug_utils_messenger_EXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *create_info,
		const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *debug_messenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, create_info, allocator, debug_messenger);

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void destroy_debug_utils_messenger_EXT(
		VkInstance instance, VkDebugUtilsMessengerEXT debug_messenger, const VkAllocationCallbacks *allocator) {
	PFN_vkDestroyDebugUtilsMessengerEXT func =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debug_messenger, allocator);
	}
}

VkInstance instance_create(const char *const *extensions, uint32_t extension_count, bool validation,
		VkDebugUtilsMessengerEXT *debug_messenger) {
	uint32_t app_version = VK_MAKE_VERSION(0, 1, 0);

	VkApplicationInfo app_info = {};
	app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName = "App";
	app_info.applicationVersion = app_version;
	app_info.pEngineName = "None";
	app_info.engineVersion = app_version;
	app_info.apiVersion = VK_API_VERSION_1_0;

	uint32_t enabled_extension_count = extension_count;
	const char **enabled_extensions = (const char **)malloc((extension_count + 1) * sizeof(const char *));

	for (uint32_t i = 0; i < extension_count; i++)
		enabled_extensions[i] = extensions[i];

	if (validation) {
		enabled_extensions[extension_count] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		enabled_extension_count += 1;
	}

	VkInstanceCreateInfo instance_info = {};
	instance_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	instance_info.pApplicationInfo = &app_info;
	instance_info.enabledExtensionCount = enabled_extension_count;
	instance_info.ppEnabledExtensionNames = enabled_extensions;

	VkDebugUtilsMessageSeverityFlagsEXT message_severity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
														   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
														   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	VkDebugUtilsMessageTypeFlagsEXT message_type = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
												   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
												   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	VkDebugUtilsMessengerCreateInfoEXT debug_info = {};
	debug_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	debug_info.messageSeverity = message_severity;
	debug_info.messageType = message_type;
	debug_info.pfnUserCallback = debug_callback;

	if (validation) {
		uint32_t enabledLayerCount = sizeof(VALIDATION_LAYERS) / sizeof(VALIDATION_LAYERS[0]);
		const char **enabledLayerNames = VALIDATION_LAYERS;

		instance_info.enabledLayerCount = enabledLayerCount;
		instance_info.ppEnabledLayerNames = enabledLayerNames;
		instance_info.pNext = &debug_info;
	}

	VkInstance instance;
	CHECK_VK_RESULT(vkCreateInstance(&instance_info, nullptr, &instance) == VK_SUCCESS, "Instance creation failed!");

	if (validation) {
		CHECK_VK_RESULT(create_debug_utils_messenger_EXT(instance, &debug_info, nullptr, debug_messenger) == VK_SUCCESS,
				"DebugUtilsMessenger creation failed!");
	}

	return instance;
}

typedef struct {
	uint32_t graphics_family;
	uint32_t present_family;
} QueueFamilyIndices;

QueueFamilyIndices find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices = {
		UINT32_MAX,
		UINT32_MAX,
	};

	uint32_t queue_family_property_count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, nullptr);

	VkQueueFamilyProperties *queue_family_properties = new VkQueueFamilyProperties[queue_family_property_count];
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_property_count, queue_family_properties);

	for (uint32_t i = 0; i < queue_family_property_count; i++) {
		if (queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}

		VkBool32 present_supported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_supported);

		if (present_supported) {
			indices.present_family = i;
		}

		if (indices.graphics_family != UINT32_MAX && indices.present_family != UINT32_MAX) {
			break;
		}
	}

	return indices;
}

bool check_device_extension_support(VkPhysicalDevice physical_device) {
	uint32_t extension_property_count = 0;
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_property_count, nullptr);

	VkExtensionProperties *extension_properties = new VkExtensionProperties[extension_property_count];
	vkEnumerateDeviceExtensionProperties(physical_device, nullptr, &extension_property_count, extension_properties);

	for (const char *extension_name : DEVICE_EXTENSIONS) {
		bool extension_found = false;

		for (uint32_t i = 0; i < extension_property_count; i++) {
			if (strcmp(extension_name, extension_properties[i].extensionName) != 0)
				continue;

			extension_found = true;
		}

		if (extension_found)
			continue;

		// not found
		return false;
	}

	return true;
}

typedef struct {
	VkSurfaceCapabilitiesKHR capabilities;
	uint32_t surface_format_count;
	VkSurfaceFormatKHR *surface_formats;
	uint32_t present_mode_count;
	VkPresentModeKHR *present_modes;
} SwapchainSupportDetails;

SwapchainSupportDetails query_swapchain_support_details(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device, surface, &capabilities);

	uint32_t present_mode_count = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, nullptr);

	VkPresentModeKHR *present_modes = new VkPresentModeKHR[present_mode_count];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physical_device, surface, &present_mode_count, present_modes);

	uint32_t surface_format_count = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, nullptr);

	VkSurfaceFormatKHR *surface_formats = new VkSurfaceFormatKHR[present_mode_count];
	vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device, surface, &surface_format_count, surface_formats);

	return { capabilities, surface_format_count, surface_formats, present_mode_count, present_modes };
}

bool is_device_suitable(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices = find_queue_families(physical_device, surface);
	bool extensions_supported = check_device_extension_support(physical_device);

	bool swapchain_adequate = false;
	if (extensions_supported) {
		SwapchainSupportDetails details = query_swapchain_support_details(physical_device, surface);
		swapchain_adequate = details.surface_format_count != 0 && details.present_mode_count != 0;
	}

	bool indices_complete = indices.graphics_family != UINT32_MAX && indices.present_family != UINT32_MAX;
	return indices_complete && extensions_supported && swapchain_adequate;
}

VkPhysicalDevice pick_physical_device(VkInstance instance, VkSurfaceKHR surface) {
	uint32_t physical_device_count = 0;
	vkEnumeratePhysicalDevices(instance, &physical_device_count, nullptr);

	VkPhysicalDevice *physical_devices = new VkPhysicalDevice[physical_device_count];
	vkEnumeratePhysicalDevices(instance, &physical_device_count, physical_devices);

	for (uint32_t i = 0; i < physical_device_count; i++) {
		if (!is_device_suitable(physical_devices[i], surface))
			continue;

		return physical_devices[i];
	}

	throw std::runtime_error("Suitable rendering device not found!");
}

VkDevice device_create(VkPhysicalDevice physical_device, VkSurfaceKHR surface, bool validation) {
	QueueFamilyIndices indices = find_queue_families(physical_device, surface);

	uint32_t queue_create_info_count = 2;
	float queue_priority = 1.0f;

	VkDeviceQueueCreateInfo queue_create_infos[2] = {};
	queue_create_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_infos[0].queueFamilyIndex = indices.graphics_family;
	queue_create_infos[0].queueCount = 1;
	queue_create_infos[0].pQueuePriorities = &queue_priority;

	queue_create_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
	queue_create_infos[1].queueFamilyIndex = indices.present_family;
	queue_create_infos[1].queueCount = 1;
	queue_create_infos[1].pQueuePriorities = &queue_priority;

	if (indices.graphics_family == indices.present_family)
		queue_create_info_count = 1;

	uint32_t enabled_extension_count = sizeof(DEVICE_EXTENSIONS) / sizeof(DEVICE_EXTENSIONS[0]);
	const char **enabled_extensions = DEVICE_EXTENSIONS;

	VkDeviceCreateInfo device_info = {};
	device_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	device_info.queueCreateInfoCount = queue_create_info_count;
	device_info.pQueueCreateInfos = queue_create_infos;
	device_info.enabledExtensionCount = enabled_extension_count;
	device_info.ppEnabledExtensionNames = enabled_extensions;

	VkDevice device;
	CHECK_VK_RESULT(
			vkCreateDevice(physical_device, &device_info, nullptr, &device) == VK_SUCCESS, "Device creation failed!");

	return device;
}

bool check_validation_layer_support() {
	uint32_t layer_property_count = 0;
	vkEnumerateInstanceLayerProperties(&layer_property_count, nullptr);

	VkLayerProperties *layer_properties = new VkLayerProperties[layer_property_count];
	vkEnumerateInstanceLayerProperties(&layer_property_count, layer_properties);

	for (const char *layer_name : VALIDATION_LAYERS) {
		bool layer_found = false;

		for (uint32_t i = 0; i < layer_property_count; i++) {
			if (strcmp(layer_name, layer_properties[i].layerName) == 0) {
				layer_found = true;
				break;
			}
		}

		if (!layer_found) {
			return false;
		}
	}

	return true;
}

VkSurfaceFormatKHR choose_surface_format(VkSurfaceFormatKHR *surface_formats, uint32_t surface_format_count) {
	assert(surface_format_count != 0 && surface_formats != nullptr);

	for (uint32_t i = 0; i < surface_format_count; i++) {
		if (surface_formats[i].format != VK_FORMAT_B8G8R8_SRGB ||
				surface_formats[i].colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			continue;

		return surface_formats[i];
	}

	return surface_formats[0];
}

VkPresentModeKHR choose_present_mode(
		VkPresentModeKHR *present_modes, uint32_t present_mode_count, VkPresentModeKHR desired_mode) {
	assert(present_mode_count != 0 && present_modes != nullptr);

	for (uint32_t i = 0; i < present_mode_count; i++) {
		if (present_modes[i] != desired_mode)
			continue;

		return present_modes[i];
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t find_memory_type(uint32_t filter, VkPhysicalDeviceMemoryProperties properties, VkMemoryPropertyFlags flags) {
	for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
		if ((filter & (1 << i)) && (properties.memoryTypes[i].propertyFlags & flags) == flags) {
			return i;
		}
	}

	throw std::runtime_error("Could not find suitable memory type!");
}

VkImage image_create(VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
		VkPhysicalDeviceMemoryProperties mem_properties, VkDeviceMemory *memory) {
	VkImageCreateInfo image_info = {};
	image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	image_info.imageType = VK_IMAGE_TYPE_2D;
	image_info.format = format;
	image_info.extent = { width, height, 1 };
	image_info.mipLevels = 1;
	image_info.arrayLayers = 1;
	image_info.samples = VK_SAMPLE_COUNT_1_BIT;
	image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
	image_info.usage = usage;
	image_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	image_info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage image;
	CHECK_VK_RESULT(vkCreateImage(device, &image_info, nullptr, &image) == VK_SUCCESS, "Image creation failed!");

	VkMemoryRequirements memory_requirements;
	vkGetImageMemoryRequirements(device, image, &memory_requirements);

	VkMemoryPropertyFlags flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
	uint32_t memory_type_index = find_memory_type(memory_requirements.memoryTypeBits, mem_properties, flags);

	VkMemoryAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	alloc_info.allocationSize = memory_requirements.size;
	alloc_info.memoryTypeIndex = memory_type_index;

	CHECK_VK_RESULT(
			vkAllocateMemory(device, &alloc_info, nullptr, memory) == VK_SUCCESS, "Image memory allocation failed!");

	vkBindImageMemory(device, image, *memory, 0);

	return image;
}

VkImageView image_view_create(VkDevice device, VkImage image, VkFormat format) {
	VkImageSubresourceRange subresource_range = {};
	subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	subresource_range.baseMipLevel = 0;
	subresource_range.levelCount = 1;
	subresource_range.baseArrayLayer = 0;
	subresource_range.layerCount = 1;

	VkImageViewCreateInfo view_info = {};
	view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	view_info.image = image;
	view_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
	view_info.format = format;
	view_info.subresourceRange = subresource_range;

	VkImageView image_view;
	CHECK_VK_RESULT(
			vkCreateImageView(device, &view_info, nullptr, &image_view) == VK_SUCCESS, "Image view creation failed!");

	return image_view;
}

void VulkanContext::_swapchain_create(uint32_t width, uint32_t height) {
	SwapchainSupportDetails details = query_swapchain_support_details(m_physical_device, m_surface);

	m_swapchain_extent = details.capabilities.currentExtent;
	if (details.capabilities.currentExtent.width == UINT32_MAX) {
		VkExtent2D min = details.capabilities.minImageExtent;
		VkExtent2D max = details.capabilities.maxImageExtent;

		uint32_t _width = vk_clamp(width, min.width, max.width);
		uint32_t _height = vk_clamp(height, min.height, max.height);

		m_swapchain_extent = { _width, _height };
	}

	uint32_t min_image_count = details.capabilities.minImageCount + 1;
	if (details.capabilities.maxImageCount > 0 && min_image_count > details.capabilities.maxImageCount)
		min_image_count = details.capabilities.maxImageCount;

	QueueFamilyIndices indices = find_queue_families(m_physical_device, m_surface);
	VkSharingMode sharing_mode = VK_SHARING_MODE_EXCLUSIVE;

	uint32_t queue_family_indices[2];

	if (indices.graphics_family != indices.present_family) {
		sharing_mode = VK_SHARING_MODE_CONCURRENT;
		queue_family_indices[0] = indices.graphics_family;
		queue_family_indices[1] = indices.present_family;
	}

	VkSurfaceFormatKHR surface_format = choose_surface_format(details.surface_formats, details.surface_format_count);
	VkPresentModeKHR present_mode =
			choose_present_mode(details.present_modes, details.present_mode_count, VK_PRESENT_MODE_MAILBOX_KHR);

	VkSwapchainCreateInfoKHR swapchain_info = {};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface = m_surface;
	swapchain_info.minImageCount = min_image_count;
	swapchain_info.imageFormat = surface_format.format;
	swapchain_info.imageColorSpace = surface_format.colorSpace;
	swapchain_info.imageExtent = m_swapchain_extent;
	swapchain_info.imageArrayLayers = 1;
	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	swapchain_info.imageSharingMode = sharing_mode;
	swapchain_info.queueFamilyIndexCount = 2;
	swapchain_info.pQueueFamilyIndices = queue_family_indices;
	swapchain_info.preTransform = details.capabilities.currentTransform;
	swapchain_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	swapchain_info.presentMode = present_mode;
	swapchain_info.clipped = VK_TRUE;

	CHECK_VK_RESULT(vkCreateSwapchainKHR(m_device, &swapchain_info, nullptr, &m_swapchain) == VK_SUCCESS,
			"Swapchain creation failed!");

	uint32_t swapchain_image_count = 0;
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, nullptr);

	VkImage *swapchain_images = new VkImage[swapchain_image_count];
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchain_image_count, swapchain_images);

	VkFormat color_format = VK_FORMAT_B10G11R11_UFLOAT_PACK32;
	m_color_image = image_create(m_device, m_swapchain_extent.width, m_swapchain_extent.height, color_format,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_memory_properties, &m_color_image_memory);

	m_color_image_view = image_view_create(m_device, m_color_image, color_format);

	VkAttachmentDescription color_attachment_description = {};
	color_attachment_description.format = surface_format.format;
	color_attachment_description.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment_description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment_description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment_description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment_description.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_reference = {};
	color_attachment_reference.attachment = 0;
	color_attachment_reference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass_description = {};
	subpass_description.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass_description.colorAttachmentCount = 1;
	subpass_description.pColorAttachments = &color_attachment_reference;

	VkRenderPassCreateInfo render_pass_info = {};
	render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	render_pass_info.attachmentCount = 1;
	render_pass_info.pAttachments = &color_attachment_description;
	render_pass_info.subpassCount = 1;
	render_pass_info.pSubpasses = &subpass_description;

	CHECK_VK_RESULT(vkCreateRenderPass(m_device, &render_pass_info, nullptr, &m_render_pass) == VK_SUCCESS,
			"Render pass creation failed!");

	m_swapchain_image_count = swapchain_image_count;
	m_swapchain_images = new SwapchainImageResource[swapchain_image_count];

	for (uint32_t i = 0; i < swapchain_image_count; i++) {
		VkImageView swapchain_view = image_view_create(m_device, swapchain_images[i], surface_format.format);

		VkFramebufferCreateInfo framebuffer_info = {};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = m_render_pass;
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = &swapchain_view;
		framebuffer_info.width = m_swapchain_extent.width;
		framebuffer_info.height = m_swapchain_extent.height;
		framebuffer_info.layers = 1;

		VkFramebuffer framebuffer;
		CHECK_VK_RESULT(vkCreateFramebuffer(m_device, &framebuffer_info, nullptr, &framebuffer) == VK_SUCCESS,
				"Swapchain framebuffer creation failed!");

		m_swapchain_images[i] = { swapchain_view, framebuffer };
	}
}

void VulkanContext::_swapchain_destroy() {
	vkDestroyImageView(m_device, m_color_image_view, nullptr);
	vkDestroyImage(m_device, m_color_image, nullptr);
	vkFreeMemory(m_device, m_color_image_memory, nullptr);

	for (uint32_t i = 0; i < m_swapchain_image_count; i++) {
		vkDestroyFramebuffer(m_device, m_swapchain_images[i].framebuffer, nullptr);
		vkDestroyImageView(m_device, m_swapchain_images[i].view, nullptr);
	}

	m_swapchain_image_count = 0;
	free(m_swapchain_images);

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	vkDestroyRenderPass(m_device, m_render_pass, nullptr);
}

void VulkanContext::create(const char *const *extensions, uint32_t extension_count, bool validation) {
	if (validation && !check_validation_layer_support()) {
		printf("Validation not supported!\n");
		validation = false;
	}

	m_validation = validation;
	m_instance = instance_create(extensions, extension_count, validation, &m_debugMessenger);
}

void VulkanContext::destroy() {
	if (m_initialized) {
		_swapchain_destroy();

		vkDestroyCommandPool(m_device, m_command_pool, nullptr);
		vkDestroyDevice(m_device, nullptr);

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}

	if (m_validation)
		destroy_debug_utils_messenger_EXT(m_instance, m_debugMessenger, nullptr);

	vkDestroyInstance(m_instance, nullptr);
}

void VulkanContext::window_create(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
	if (m_initialized)
		return;

	m_surface = surface;

	m_physical_device = pick_physical_device(m_instance, m_surface);
	vkGetPhysicalDeviceMemoryProperties(m_physical_device, &m_memory_properties);

	m_device = device_create(m_physical_device, m_surface, m_validation);

	QueueFamilyIndices indices = find_queue_families(m_physical_device, m_surface);
	vkGetDeviceQueue(m_device, indices.graphics_family, 0, &m_graphics_queue);
	vkGetDeviceQueue(m_device, indices.present_family, 0, &m_present_queue);

	m_graphics_queue_family = indices.graphics_family;

	_swapchain_create(width, height);

	VkCommandPoolCreateInfo command_pool_info = {};
	command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	command_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	command_pool_info.queueFamilyIndex = indices.graphics_family;

	CHECK_VK_RESULT(vkCreateCommandPool(m_device, &command_pool_info, nullptr, &m_command_pool) == VK_SUCCESS,
			"CommandPool creation failed!");

	m_initialized = true;
}

void VulkanContext::window_resize(uint32_t width, uint32_t height) {
	vkDeviceWaitIdle(m_device);

	_swapchain_destroy();
	_swapchain_create(width, height);
}
