#ifndef ALLOCATED_H
#define ALLOCATED_H

#include <cstddef>

typedef struct VmaAllocation_T *VmaAllocation;
typedef struct VkBuffer_T *VkBuffer;

typedef struct Allocation {
	VmaAllocation handle;
	size_t size;
} Allocation;

typedef struct AllocatedBuffer {
	Allocation allocation;
	VkBuffer buffer;
} AllocatedBuffer;

#endif // !ALLOCATED_H
