#ifndef MESH_STORAGE_H
#define MESH_STORAGE_H

class MaterialStorage {
public:
	static MaterialStorage &singleton() {
		static MaterialStorage instance;
		return instance;
	}

	MaterialStorage(MaterialStorage const &) = delete;
	void operator=(MaterialStorage const &) = delete;

private:
	MaterialStorage() {}

public:
};

#endif // !MESH_STORAGE_H
