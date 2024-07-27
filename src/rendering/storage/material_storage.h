#ifndef MATERIAL_STORAGE_H
#define MATERIAL_STORAGE_H

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

#endif // !MATERIAL_STORAGE_H
