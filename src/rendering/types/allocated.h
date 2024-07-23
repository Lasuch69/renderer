#ifndef ALLOCATED_H
#define ALLOCATED_H

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

#endif // !ALLOCATED_H
