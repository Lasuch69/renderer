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
	VkInstance instance() const;
	VkSurfaceKHR surface() const;
	VkPhysicalDevice physicalDevice() const;
	VkPhysicalDeviceMemoryProperties memoryProperties() const;
	VkDevice device() const;
	VkQueue graphicsQueue() const;
	VkQueue presentQueue() const;
	uint32_t graphicsQueueFamily() const;
	VkSwapchainKHR swapchain() const;
	VkExtent2D swapchainExtent() const;
	VkRenderPass renderPass() const;
	VkFramebuffer framebuffer(uint32_t imageIndex) const;
	VkCommandPool commandPool() const;

	bool isInitialized() const;

	void create(const char *const *extensions, uint32_t extensionCount, bool validation);
	void destroy();

	void windowCreate(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void windowResize(uint32_t width, uint32_t height);
};

#endif // !VULKAN_CONTEXT_H
