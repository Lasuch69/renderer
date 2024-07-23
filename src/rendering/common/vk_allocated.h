#ifndef VK_ALLOCATED_H
#define VK_ALLOCATED_H

typedef struct VmaAllocation_T *VmaAllocation;
typedef struct VkBuffer_T *VkBuffer;
typedef struct VkImage_T *VkImage;

typedef struct {
	VmaAllocation allocation;
	VkBuffer handle;
} AllocatedBuffer;

typedef struct {
	VmaAllocation allocation;
	VkImage handle;
} AllocatedImage;

#endif // !VK_ALLOCATED_H
