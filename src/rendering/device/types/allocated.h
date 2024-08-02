#ifndef ALLOCATED_H
#define ALLOCATED_H

#include <cstddef>
#include <functional>

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

namespace std {
template <> struct hash<AllocatedBuffer> {
	size_t operator()(const AllocatedBuffer &v) const {
		return ((size_t)v.buffer >> 1) * ((size_t)v.allocation.handle << 1);
	}
};
} // namespace std

#endif // !ALLOCATED_H
