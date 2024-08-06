#ifndef RESOURCE_OWNER_H
#define RESOURCE_OWNER_H

#include <cstdint>
#include <unordered_map>

#define NULL_HANDLE 0

template <typename value> class ResourceOwner {
private:
	std::unordered_map<uint64_t, value> m_map;
	uint64_t m_idx = 0;

public:
	inline uint64_t insert(const value &v) {
		m_idx++;
		m_map[m_idx] = v;
		return m_idx;
	}

	inline void remove(uint64_t id) {
		m_map.erase(id);
	}

	inline value *get(uint64_t id) {
		return (m_map.count(id) != 0) && (id != NULL_HANDLE) ? &m_map[id] : nullptr;
	}

	inline value *get(uint64_t id) const {
		return (m_map.count(id) != 0) && (id != NULL_HANDLE) ? &m_map[id] : nullptr;
	}

	inline bool has(uint64_t id) const {
		return (m_map.count(id) != 0) && (id != NULL_HANDLE);
	}
};

#endif // !RESOURCE_OWNER_H
