#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include <vulkan/vulkan_core.h>

class VulkanContext {
private:
	bool m_validation = false;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkSurfaceKHR m_surface;

	VkPhysicalDevice m_physicalDevice;
	VkPhysicalDeviceMemoryProperties m_memoryProperties;

	VkDevice m_device;

	VkQueue m_graphicsQueue;
	VkQueue m_presentQueue;

	uint32_t m_graphicsQueueFamily;

	typedef struct {
		VkImageView view;
		VkFramebuffer framebuffer;
	} SwapchainImageResource;

	uint32_t m_swapchainImageCount;
	SwapchainImageResource *m_swapchainImages;

	VkSwapchainKHR m_swapchain;
	VkExtent2D m_swapchainExtent;
	VkRenderPass m_renderPass;

	VkImage m_colorImage;
	VkDeviceMemory m_colorImageMemory;
	VkImageView m_colorImageView;

	VkCommandPool m_commandPool;

	bool m_initialized = false;

	void _swapchainCreate(uint32_t width, uint32_t height);
	void _swapchainDestroy();

public:
	inline VkInstance instance() const {
		return m_instance;
	}

	inline VkSurfaceKHR surface() const {
		return m_surface;
	}

	inline VkPhysicalDevice physicalDevice() const {
		return m_physicalDevice;
	}

	inline VkPhysicalDeviceMemoryProperties memoryProperties() const {
		return m_memoryProperties;
	}

	inline VkDevice device() const {
		return m_device;
	}

	inline VkQueue graphicsQueue() const {
		return m_graphicsQueue;
	}

	inline VkQueue presentQueue() const {
		return m_presentQueue;
	}

	inline uint32_t graphicsQueueFamily() const {
		return m_graphicsQueueFamily;
	}

	inline VkSwapchainKHR swapchain() const {
		return m_swapchain;
	}

	inline VkExtent2D swapchainExtent() const {
		return m_swapchainExtent;
	}

	inline VkRenderPass renderPass() const {
		return m_renderPass;
	}

	inline VkFramebuffer framebuffer(uint32_t imageIndex) const {
		return m_swapchainImages[imageIndex].framebuffer;
	}

	inline VkCommandPool commandPool() const {
		return m_commandPool;
	}

	inline bool initialized() const {
		return m_initialized;
	}

	void create(const char *const *extensions, uint32_t extensionCount, bool validation);
	void destroy();

	void windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void windowResize(uint32_t width, uint32_t height);
};

#endif // !VULKAN_CONTEXT_H
