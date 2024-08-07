#ifndef VULKAN_CONTEXT_H
#define VULKAN_CONTEXT_H

#include <vulkan/vulkan_core.h>

class VulkanContext {
private:
	bool m_validation = false;

	VkInstance m_instance;
	VkDebugUtilsMessengerEXT m_debugMessenger;

	VkSurfaceKHR m_surface;

	VkPhysicalDevice m_physical_device;
	VkPhysicalDeviceMemoryProperties m_memory_properties;

	VkDevice m_device;

	VkQueue m_graphics_queue;
	VkQueue m_present_queue;

	uint32_t m_graphics_queue_family;

	typedef struct {
		VkImageView view;
		VkFramebuffer framebuffer;
	} SwapchainImageResource;

	uint32_t m_swapchain_image_count;
	SwapchainImageResource *m_swapchain_images;

	VkSwapchainKHR m_swapchain;
	VkExtent2D m_swapchain_extent;
	VkRenderPass m_render_pass;

	VkImage m_color_image;
	VkDeviceMemory m_color_image_memory;
	VkImageView m_color_image_view;

	VkCommandPool m_command_pool;

	bool m_initialized = false;

	void _swapchain_create(uint32_t width, uint32_t height);
	void _swapchain_destroy();

public:
	inline VkInstance instance() const {
		return m_instance;
	}

	inline VkSurfaceKHR surface() const {
		return m_surface;
	}

	inline VkPhysicalDevice physical_device() const {
		return m_physical_device;
	}

	inline VkPhysicalDeviceMemoryProperties memory_properties() const {
		return m_memory_properties;
	}

	inline VkDevice device() const {
		return m_device;
	}

	inline VkQueue graphics_queue() const {
		return m_graphics_queue;
	}

	inline VkQueue present_queue() const {
		return m_present_queue;
	}

	inline uint32_t graphics_queue_family() const {
		return m_graphics_queue_family;
	}

	inline VkSwapchainKHR swapchain() const {
		return m_swapchain;
	}

	inline VkExtent2D swapchain_extent() const {
		return m_swapchain_extent;
	}

	inline VkRenderPass render_pass() const {
		return m_render_pass;
	}

	inline VkFramebuffer framebuffer(uint32_t imageIndex) const {
		return m_swapchain_images[imageIndex].framebuffer;
	}

	inline VkCommandPool command_pool() const {
		return m_command_pool;
	}

	inline bool initialized() const {
		return m_initialized;
	}

	void create(const char *const *extensions, uint32_t extensionCount, bool validation);
	void destroy();

	void window_create(VkSurfaceKHR surface, uint32_t width, uint32_t height);
	void window_resize(uint32_t width, uint32_t height);
};

#endif // !VULKAN_CONTEXT_H
