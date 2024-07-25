#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

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

typedef struct {
	VkSurfaceCapabilitiesKHR capabilities;
	uint32_t surfaceFormatCount;
	VkSurfaceFormatKHR *surfaceFormats;
	uint32_t presentModeCount;
	VkPresentModeKHR *presentModes;
} SwapchainSupportDetails;

struct QueueFamilyIndices {
	uint32_t graphicsFamily = UINT32_MAX;
	uint32_t presentFamily = UINT32_MAX;

	bool isComplete() {
		return graphicsFamily != UINT32_MAX && presentFamily != UINT32_MAX;
	}
};

static uint32_t max(uint32_t a, uint32_t b) {
	return a > b ? a : b;
}

static uint32_t min(uint32_t a, uint32_t b) {
	return a < b ? a : b;
}

static uint32_t clamp(uint32_t value, uint32_t minimum, uint32_t maximum) {
	return max(minimum, min(value, maximum));
}

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *createInfo,
		const VkAllocationCallbacks *allocator, VkDebugUtilsMessengerEXT *debugMessenger) {
	PFN_vkCreateDebugUtilsMessengerEXT func =
			(PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
		return func(instance, createInfo, allocator, debugMessenger);

	return VK_ERROR_EXTENSION_NOT_PRESENT;
}

void DestroyDebugUtilsMessengerEXT(
		VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *allocator) {
	PFN_vkDestroyDebugUtilsMessengerEXT func =
			(PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
	if (func != nullptr) {
		func(instance, debugMessenger, allocator);
	}
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT *callbackData,
		void *pUserData) {
	if (messageSeverity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
		return VK_FALSE;
	}

	printf("%s\n", callbackData->pMessage);
	return VK_FALSE;
}

VkInstance instanceCreate(const char *const *extensions, uint32_t extensionCount, bool validation,
		VkDebugUtilsMessengerEXT *debugMessenger) {
	uint32_t appVersion = VK_MAKE_VERSION(0, 1, 0);

	VkApplicationInfo appInfo = {
		.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName = "App",
		.applicationVersion = appVersion,
		.pEngineName = "None",
		.engineVersion = appVersion,
		.apiVersion = VK_API_VERSION_1_0,
	};

	uint32_t enabledExtensionCount = extensionCount;
	const char **enabledExtensions = (const char **)malloc((extensionCount + 1) * sizeof(const char *));

	for (uint32_t i = 0; i < extensionCount; i++)
		enabledExtensions[i] = extensions[i];

	if (validation) {
		enabledExtensions[extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		enabledExtensionCount += 1;
	}

	VkInstanceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo = &appInfo,
		.enabledExtensionCount = enabledExtensionCount,
		.ppEnabledExtensionNames = enabledExtensions,
	};

	VkDebugUtilsMessageSeverityFlagsEXT messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
														  VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
														  VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	VkDebugUtilsMessageTypeFlagsEXT messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
												  VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
												  VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity = messageSeverity,
		.messageType = messageType,
		.pfnUserCallback = debugCallback,
	};

	if (validation) {
		uint32_t enabledLayerCount = sizeof(VALIDATION_LAYERS) / sizeof(VALIDATION_LAYERS[0]);
		const char **enabledLayerNames = VALIDATION_LAYERS;

		createInfo.enabledLayerCount = enabledLayerCount;
		createInfo.ppEnabledLayerNames = enabledLayerNames;
		createInfo.pNext = &debugCreateInfo;
	}

	VkInstance instance;
	CHECK_VK_RESULT(vkCreateInstance(&createInfo, nullptr, &instance) == VK_SUCCESS, "Instance creation failed!");

	if (validation) {
		CHECK_VK_RESULT(CreateDebugUtilsMessengerEXT(instance, &debugCreateInfo, nullptr, debugMessenger) == VK_SUCCESS,
				"DebugUtilsMessenger creation failed!");
	}

	return instance;
}

QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;

	uint32_t queueFamilyPropertyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, nullptr);

	VkQueueFamilyProperties *queueFamilyProperties = new VkQueueFamilyProperties[queueFamilyPropertyCount];
	vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyPropertyCount, queueFamilyProperties);

	for (uint32_t i = 0; i < queueFamilyPropertyCount; i++) {
		if (queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupported = VK_FALSE;
		vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface, &presentSupported);

		if (presentSupported) {
			indices.presentFamily = i;
		}

		if (indices.isComplete()) {
			break;
		}
	}

	return indices;
}

bool checkDeviceExtensionSupport(VkPhysicalDevice physicalDevice) {
	uint32_t extensionPropertyCount = 0;
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionPropertyCount, nullptr);

	VkExtensionProperties *extensionProperties = new VkExtensionProperties[extensionPropertyCount];
	vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionPropertyCount, extensionProperties);

	for (const char *extensionName : DEVICE_EXTENSIONS) {
		bool extensionFound = false;

		for (uint32_t i = 0; i < extensionPropertyCount; i++) {
			if (strcmp(extensionName, extensionProperties[i].extensionName) != 0)
				continue;

			extensionFound = true;
		}

		if (extensionFound)
			continue;

		// not found
		return false;
	}

	return true;
}

SwapchainSupportDetails querySwapchainSupportDetails(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	VkSurfaceCapabilitiesKHR capabilities;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &capabilities);

	uint32_t presentModeCount = 0;
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, nullptr);

	VkPresentModeKHR *presentModes = new VkPresentModeKHR[presentModeCount];
	vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice, surface, &presentModeCount, presentModes);

	uint32_t surfaceFormatCount = 0;
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, nullptr);

	VkSurfaceFormatKHR *surfaceFormats = new VkSurfaceFormatKHR[presentModeCount];
	vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice, surface, &surfaceFormatCount, surfaceFormats);

	SwapchainSupportDetails details = {
		.capabilities = capabilities,
		.surfaceFormatCount = surfaceFormatCount,
		.surfaceFormats = surfaceFormats,
		.presentModeCount = presentModeCount,
		.presentModes = presentModes,
	};

	return details;
}

bool isDeviceSuitable(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
	bool extensionsSupported = checkDeviceExtensionSupport(physicalDevice);

	bool swapchainAdequate = false;
	if (extensionsSupported) {
		SwapchainSupportDetails details = querySwapchainSupportDetails(physicalDevice, surface);
		swapchainAdequate = details.surfaceFormatCount != 0 && details.presentModeCount != 0;
	}

	return indices.isComplete() && extensionsSupported && swapchainAdequate;
}

VkPhysicalDevice pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface) {
	uint32_t physicalDeviceCount = 0;
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, nullptr);

	VkPhysicalDevice *physicalDevices = new VkPhysicalDevice[physicalDeviceCount];
	vkEnumeratePhysicalDevices(instance, &physicalDeviceCount, physicalDevices);

	for (uint32_t i = 0; i < physicalDeviceCount; i++) {
		if (!isDeviceSuitable(physicalDevices[i], surface))
			continue;

		return physicalDevices[i];
	}

	printf("Suitable rendering device not found!\n");
	return VK_NULL_HANDLE;
}

VkDevice deviceCreate(VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, bool validation) {
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);

	uint32_t queueCreateInfoCount = 2;
	float queuePriority = 1.0f;
	VkDeviceQueueCreateInfo queueCreateInfos[2];

	queueCreateInfos[0] = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = indices.graphicsFamily,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority,
	};

	queueCreateInfos[1] = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueFamilyIndex = indices.presentFamily,
		.queueCount = 1,
		.pQueuePriorities = &queuePriority,
	};

	if (indices.graphicsFamily == indices.presentFamily)
		queueCreateInfoCount = 1;

	uint32_t enabledExtensionCount = sizeof(DEVICE_EXTENSIONS) / sizeof(DEVICE_EXTENSIONS[0]);
	const char **enabledExtensions = DEVICE_EXTENSIONS;

	VkDeviceCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount = queueCreateInfoCount,
		.pQueueCreateInfos = queueCreateInfos,
		.enabledExtensionCount = enabledExtensionCount,
		.ppEnabledExtensionNames = enabledExtensions,
	};

	VkDevice device;
	CHECK_VK_RESULT(
			vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) == VK_SUCCESS, "Device creation failed!");

	return device;
}

bool checkValidationLayerSupport() {
	uint32_t layerPropertyCount = 0;
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, nullptr);

	VkLayerProperties *layerProperties = new VkLayerProperties[layerPropertyCount];
	vkEnumerateInstanceLayerProperties(&layerPropertyCount, layerProperties);

	for (const char *layerName : VALIDATION_LAYERS) {
		bool layerFound = false;

		for (uint32_t i = 0; i < layerPropertyCount; i++) {
			if (strcmp(layerName, layerProperties[i].layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			return false;
		}
	}

	return true;
}

VkSurfaceFormatKHR chooseSurfaceFormat(VkSurfaceFormatKHR *surfaceFormats, uint32_t surfaceFormatCount) {
	assert(surfaceFormatCount != 0 && surfaceFormats != nullptr);

	for (uint32_t i = 0; i < surfaceFormatCount; i++) {
		if (surfaceFormats[i].format != VK_FORMAT_B8G8R8_SRGB ||
				surfaceFormats[i].colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			continue;

		return surfaceFormats[i];
	}

	return surfaceFormats[0];
}

VkPresentModeKHR choosePresentMode(
		VkPresentModeKHR *presentModes, uint32_t presentModeCount, VkPresentModeKHR desiredMode) {
	assert(presentModeCount != 0 && presentModes != nullptr);

	for (uint32_t i = 0; i < presentModeCount; i++) {
		if (presentModes[i] != desiredMode)
			continue;

		return presentModes[i];
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

uint32_t findMemoryType(uint32_t filter, VkPhysicalDeviceMemoryProperties properties, VkMemoryPropertyFlags flags) {
	for (uint32_t i = 0; i < properties.memoryTypeCount; i++) {
		if ((filter & (1 << i)) && (properties.memoryTypes[i].propertyFlags & flags) == flags) {
			return i;
		}
	}

	printf("Could not find suitable memory type!\n");
	return 0;
}

VkImage imageCreate(VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
		VkPhysicalDeviceMemoryProperties memProperties, VkDeviceMemory *memory) {
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

	VkImage image;
	CHECK_VK_RESULT(vkCreateImage(device, &createInfo, nullptr, &image) == VK_SUCCESS, "Image creation failed!");

	VkMemoryRequirements memoryRequirements;
	vkGetImageMemoryRequirements(device, image, &memoryRequirements);

	uint32_t memoryTypeIndex =
			findMemoryType(memoryRequirements.memoryTypeBits, memProperties, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryAllocateInfo allocInfo = {
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = memoryRequirements.size,
		.memoryTypeIndex = memoryTypeIndex,
	};

	CHECK_VK_RESULT(
			vkAllocateMemory(device, &allocInfo, nullptr, memory) == VK_SUCCESS, "Image memory allocation failed!");

	vkBindImageMemory(device, image, *memory, 0);

	return image;
}

VkImageView imageViewCreate(VkDevice device, VkImage image, VkFormat format, VkImageAspectFlags aspectMask) {
	VkImageSubresourceRange subresourceRange = {
		.aspectMask = aspectMask,
		.baseMipLevel = 0,
		.levelCount = 1,
		.baseArrayLayer = 0,
		.layerCount = 1,
	};

	VkImageViewCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		.image = image,
		.viewType = VK_IMAGE_VIEW_TYPE_2D,
		.format = format,
		.subresourceRange = subresourceRange,
	};

	VkImageView imageView;
	CHECK_VK_RESULT(
			vkCreateImageView(device, &createInfo, nullptr, &imageView) == VK_SUCCESS, "Image view creation failed!");

	return imageView;
}

void attachmentCreate(VkDevice device, uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage,
		VkImageAspectFlags aspectMask, VkPhysicalDeviceMemoryProperties memProperties, Attachment *attachment) {
	attachment->image = imageCreate(device, width, height, format, usage, memProperties, &attachment->imageMemory);
	attachment->imageView = imageViewCreate(device, attachment->image, format, aspectMask);
}

void attachmentDestroy(VkDevice device, Attachment *attachment) {
	vkDestroyImageView(device, attachment->imageView, nullptr);
	vkDestroyImage(device, attachment->image, nullptr);
	vkFreeMemory(device, attachment->imageMemory, nullptr);
}

void VulkanContext::_swapchainCreate(uint32_t width, uint32_t height) {
	SwapchainSupportDetails details = querySwapchainSupportDetails(m_physicalDevice, m_surface);

	m_swapchainExtent = details.capabilities.currentExtent;
	if (details.capabilities.currentExtent.width == UINT32_MAX) {
		VkExtent2D min = details.capabilities.minImageExtent;
		VkExtent2D max = details.capabilities.maxImageExtent;

		uint32_t _width = clamp(width, min.width, max.width);
		uint32_t _height = clamp(height, min.height, max.height);

		m_swapchainExtent = { _width, _height };
	}

	uint32_t minImageCount = details.capabilities.minImageCount + 1;
	if (details.capabilities.maxImageCount > 0 && minImageCount > details.capabilities.maxImageCount) {
		minImageCount = details.capabilities.maxImageCount;
	}

	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);
	VkSharingMode sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	uint32_t queueFamilyIndices[2];

	if (indices.graphicsFamily != indices.presentFamily) {
		sharingMode = VK_SHARING_MODE_CONCURRENT;
		queueFamilyIndices[0] = indices.graphicsFamily;
		queueFamilyIndices[1] = indices.presentFamily;
	}

	VkSurfaceFormatKHR surfaceFormat = chooseSurfaceFormat(details.surfaceFormats, details.surfaceFormatCount);
	VkPresentModeKHR presentMode =
			choosePresentMode(details.presentModes, details.presentModeCount, VK_PRESENT_MODE_MAILBOX_KHR);

	VkSwapchainCreateInfoKHR createInfo = {
		.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface = m_surface,
		.minImageCount = minImageCount,
		.imageFormat = surfaceFormat.format,
		.imageColorSpace = surfaceFormat.colorSpace,
		.imageExtent = m_swapchainExtent,
		.imageArrayLayers = 1,
		.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode = sharingMode,
		.queueFamilyIndexCount = 2,
		.pQueueFamilyIndices = queueFamilyIndices,
		.preTransform = details.capabilities.currentTransform,
		.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode = presentMode,
		.clipped = VK_TRUE,
	};

	CHECK_VK_RESULT(vkCreateSwapchainKHR(m_device, &createInfo, nullptr, &m_swapchain) == VK_SUCCESS,
			"Swapchain creation failed!");

	uint32_t swapchainImageCount = 0;
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, nullptr);

	VkImage *swapchainImages = new VkImage[swapchainImageCount];
	vkGetSwapchainImagesKHR(m_device, m_swapchain, &swapchainImageCount, swapchainImages);

	uint32_t _width = m_swapchainExtent.width;
	uint32_t _height = m_swapchainExtent.height;

	VkFormat albedoFormat = VK_FORMAT_R8G8B8A8_SRGB;
	attachmentCreate(m_device, _width, _height, albedoFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
			m_memoryProperties, &m_albedoAttachment);

	VkFormat normalFormat = VK_FORMAT_A2R10G10B10_UNORM_PACK32;
	attachmentCreate(m_device, _width, _height, normalFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
			m_memoryProperties, &m_normalAttachment);

	VkFormat roughnessMetallicFormat = VK_FORMAT_R8G8_UNORM;
	attachmentCreate(m_device, _width, _height, roughnessMetallicFormat,
			VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT, VK_IMAGE_ASPECT_COLOR_BIT,
			m_memoryProperties, &m_roughnessMetallicAttachment);

	VkFormat depthFormat = VK_FORMAT_D32_SFLOAT;
	attachmentCreate(m_device, _width, _height, depthFormat,
			VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT,
			VK_IMAGE_ASPECT_DEPTH_BIT, m_memoryProperties, &m_depthAttachment);

	VkAttachmentDescription finalColorDescription = {
		.format = surfaceFormat.format,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
	};

	VkAttachmentDescription albedoDescription = {
		.format = albedoFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentDescription normalDescription = {
		.format = normalFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentDescription roughnessMetallicDescription = {
		.format = roughnessMetallicFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentDescription depthDescription = {
		.format = depthFormat,
		.samples = VK_SAMPLE_COUNT_1_BIT,
		.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
		.storeOp = VK_ATTACHMENT_STORE_OP_STORE,
		.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
		.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference finalColorReference = {
		.attachment = 0,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference albedoReference = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference normalReference = {
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference roughnessMetallicReference = {
		.attachment = 3,
		.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference depthReference = {
		.attachment = 4,
		.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkAttachmentReference albedoInputReference = {
		.attachment = 1,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkAttachmentReference normalInputReference = {
		.attachment = 2,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkAttachmentReference roughnessMetallicInputReference = {
		.attachment = 3,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkAttachmentReference depthInputReference = {
		.attachment = 4,
		.layout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
	};

	VkAttachmentReference geometryColorAttachments[] = {
		albedoReference,
		normalReference,
		roughnessMetallicReference,
	};

	uint32_t geometryColorAttachmentCount = sizeof(geometryColorAttachments) / sizeof(geometryColorAttachments[0]);

	VkSubpassDescription geometrySubpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.colorAttachmentCount = geometryColorAttachmentCount,
		.pColorAttachments = geometryColorAttachments,
		.pDepthStencilAttachment = &depthReference,
	};

	VkAttachmentReference lightInputAttachments[] = {
		albedoInputReference,
		normalInputReference,
		roughnessMetallicInputReference,
		depthInputReference,
	};

	uint32_t lightInputAttachmentCount = sizeof(lightInputAttachments) / sizeof(lightInputAttachments[0]);

	VkSubpassDescription lightSubpass = {
		.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
		.inputAttachmentCount = lightInputAttachmentCount,
		.pInputAttachments = lightInputAttachments,
		.colorAttachmentCount = 1,
		.pColorAttachments = &finalColorReference,
	};

	VkSubpassDescription subpasses[] = {
		geometrySubpass,
		lightSubpass,
	};

	uint32_t subpassCount = sizeof(subpasses) / sizeof(subpasses[0]);

	VkAttachmentDescription renderPassAttachments[] = {
		finalColorDescription,
		albedoDescription,
		normalDescription,
		roughnessMetallicDescription,
		depthDescription,
	};

	uint32_t renderPassAttachmentCount = sizeof(renderPassAttachments) / sizeof(renderPassAttachments[0]);

	VkSubpassDependency dependency = {
		.srcSubpass = GEOMETRY_PASS,
		.dstSubpass = LIGHT_PASS,
		.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		.dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		.srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		.dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
	};

	VkRenderPassCreateInfo renderPassInfo = {
		.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		.attachmentCount = renderPassAttachmentCount,
		.pAttachments = renderPassAttachments,
		.subpassCount = subpassCount,
		.pSubpasses = subpasses,
		.dependencyCount = 1,
		.pDependencies = &dependency,
	};

	CHECK_VK_RESULT(vkCreateRenderPass(m_device, &renderPassInfo, nullptr, &m_renderPass) == VK_SUCCESS,
			"Render pass creation failed!");

	m_swapchainImageCount = swapchainImageCount;
	m_swapchainImages = new SwapchainImageResource[swapchainImageCount];

	for (uint32_t i = 0; i < swapchainImageCount; i++) {
		VkImageView swapchainView =
				imageViewCreate(m_device, swapchainImages[i], surfaceFormat.format, VK_IMAGE_ASPECT_COLOR_BIT);

		VkImageView attachments[] = {
			swapchainView,
			m_albedoAttachment.imageView,
			m_normalAttachment.imageView,
			m_roughnessMetallicAttachment.imageView,
			m_depthAttachment.imageView,
		};

		uint32_t attachmentCount = sizeof(attachments) / sizeof(attachments[0]);

		VkFramebufferCreateInfo framebufferInfo = {
			.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.renderPass = m_renderPass,
			.attachmentCount = attachmentCount,
			.pAttachments = attachments,
			.width = m_swapchainExtent.width,
			.height = m_swapchainExtent.height,
			.layers = 1,
		};

		VkFramebuffer framebuffer;
		CHECK_VK_RESULT(vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &framebuffer) == VK_SUCCESS,
				"Swapchain framebuffer creation failed!");

		m_swapchainImages[i] = { swapchainView, framebuffer };
	}
}

void VulkanContext::_swapchainDestroy() {
	attachmentDestroy(m_device, &m_albedoAttachment);
	attachmentDestroy(m_device, &m_normalAttachment);
	attachmentDestroy(m_device, &m_roughnessMetallicAttachment);
	attachmentDestroy(m_device, &m_depthAttachment);

	for (uint32_t i = 0; i < m_swapchainImageCount; i++) {
		vkDestroyFramebuffer(m_device, m_swapchainImages[i].framebuffer, nullptr);
		vkDestroyImageView(m_device, m_swapchainImages[i].view, nullptr);
	}

	m_swapchainImageCount = 0;
	free(m_swapchainImages);

	vkDestroySwapchainKHR(m_device, m_swapchain, nullptr);
	vkDestroyRenderPass(m_device, m_renderPass, nullptr);
}

VkInstance VulkanContext::instance() const {
	return m_instance;
}

VkSurfaceKHR VulkanContext::surface() const {
	return m_surface;
}

VkPhysicalDevice VulkanContext::physicalDevice() const {
	return m_physicalDevice;
}

VkPhysicalDeviceMemoryProperties VulkanContext::memoryProperties() const {
	return m_memoryProperties;
}

VkDevice VulkanContext::device() const {
	return m_device;
}

VkQueue VulkanContext::graphicsQueue() const {
	return m_graphicsQueue;
}

VkQueue VulkanContext::presentQueue() const {
	return m_presentQueue;
}

uint32_t VulkanContext::graphicsQueueFamily() const {
	return m_graphicsQueueFamily;
}

VkSwapchainKHR VulkanContext::swapchain() const {
	return m_swapchain;
}

VkExtent2D VulkanContext::swapchainExtent() const {
	return m_swapchainExtent;
}

VkRenderPass VulkanContext::renderPass() const {
	return m_renderPass;
}

VkFramebuffer VulkanContext::framebuffer(uint32_t imageIndex) const {
	return m_swapchainImages[imageIndex].framebuffer;
}

VkCommandPool VulkanContext::commandPool() const {
	return m_commandPool;
}

Attachment VulkanContext::albedoAttachment() const {
	return m_albedoAttachment;
}

Attachment VulkanContext::normalAttachment() const {
	return m_normalAttachment;
}

Attachment VulkanContext::roughnessMetallicAttachment() const {
	return m_roughnessMetallicAttachment;
}

Attachment VulkanContext::depthAttachment() const {
	return m_depthAttachment;
}

bool VulkanContext::isInitialized() const {
	return m_initialized;
}

void VulkanContext::create(const char *const *extensions, uint32_t extensionCount, bool validation) {
	if (validation && !checkValidationLayerSupport()) {
		printf("Validation not supported!\n");
		validation = false;
	}

	m_validation = validation;
	m_instance = instanceCreate(extensions, extensionCount, validation, &m_debugMessenger);
}

void VulkanContext::destroy() {
	if (m_initialized) {
		_swapchainDestroy();

		vkDestroyCommandPool(m_device, m_commandPool, nullptr);
		vkDestroyDevice(m_device, nullptr);

		vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
	}

	if (m_validation)
		DestroyDebugUtilsMessengerEXT(m_instance, m_debugMessenger, nullptr);

	vkDestroyInstance(m_instance, nullptr);
}

void VulkanContext::windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height) {
	if (m_initialized)
		return;

	m_surface = surface;

	m_physicalDevice = pickPhysicalDevice(m_instance, m_surface);
	vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &m_memoryProperties);

	m_device = deviceCreate(m_physicalDevice, m_surface, m_validation);

	QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice, m_surface);
	vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
	vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);

	m_graphicsQueueFamily = indices.graphicsFamily;

	_swapchainCreate(width, height);

	VkCommandPoolCreateInfo commandPoolCreateInfo = {
		.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
		.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
		.queueFamilyIndex = indices.graphicsFamily,
	};

	CHECK_VK_RESULT(vkCreateCommandPool(m_device, &commandPoolCreateInfo, nullptr, &m_commandPool) == VK_SUCCESS,
			"CommandPool creation failed!");

	m_initialized = true;
}

void VulkanContext::windowResize(uint32_t width, uint32_t height) {
	vkDeviceWaitIdle(m_device);

	_swapchainDestroy();
	_swapchainCreate(width, height);
}
