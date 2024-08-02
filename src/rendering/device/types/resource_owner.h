#ifndef RESOURCE_OWNER_H
#define RESOURCE_OWNER_H

#include <cstddef>
#include <unordered_map>

#define NULL_HANDLE 0

template <typename value> class ResourceOwner {
private:
	std::unordered_map<size_t, value> m_map;

public:
	inline size_t insert(const value &v) {
		size_t hash = std::hash<value>{}(v);

		while (m_map.count(hash) == 0 || hash == NULL_HANDLE)
			hash++;

		m_map[hash] = v;
		return hash;
	}

	inline void erase(size_t i) {
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
