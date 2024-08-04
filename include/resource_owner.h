#ifndef RESOURCE_OWNER_H
#define RESOURCE_OWNER_H

#include <cstddef>
#include <unordered_map>

#define NULL_HANDLE 0

template <typename value> class ResourceOwner {
private:
	std::unordered_map<size_t, value> m_map;
	size_t m_idx = 0;

public:
	inline size_t insert(const value &v) {
		m_idx++;
		m_map[m_idx] = v;
		return m_idx;
	}

	inline void remove(size_t i) {
		m_map.erase(i);
	}

	inline value *get(size_t i) {
		return (m_map.count(i) != 0) && (i != NULL_HANDLE) ? &m_map[i] : nullptr;
	}

	inline value *get(size_t i) const {
		return (m_map.count(i) != 0) && (i != NULL_HANDLE) ? &m_map[i] : nullptr;
	}

	inline bool has(size_t i) const {
		return (m_map.count(i) != 0) && (i != NULL_HANDLE);
	}
};

#endif // !RESOURCE_OWNER_H
