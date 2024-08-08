#ifndef RESOURCE_OWNER_H
#define RESOURCE_OWNER_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>
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

	inline value **list(size_t *size) const {
		size_t count = 0;
		for (auto pair : m_map) {
			count++;
		}

		if (count == 0) {
			*size = count;
			return nullptr;
		}

		value **list = (value **)malloc(*size * sizeof(value *));

		size_t i = 0;
		for (auto pair : m_map) {
			list[i] = &pair.second;
			i++;
		}

		return list;
	}

	inline bool has(uint64_t id) const {
		return (m_map.count(id) != 0) && (id != NULL_HANDLE);
	}
};

#endif // !RESOURCE_OWNER_H
