#ifndef LIGHT_STORAGE_H
#define LIGHT_STORAGE_H

typedef struct {
	float color[3];
	float intensity;
	float direction[3];
	float _padding;
} DirectionalLight;
static_assert(sizeof(DirectionalLight) % 16 == 0, "DirectionalLight is not multiple of 16");

typedef struct {
	float color[3];
	float intensity;
	float position[3];
	float range;
} PunctualLight;
static_assert(sizeof(PunctualLight) % 16 == 0, "PunctualLight is not multiple of 16");

class LightStorage {
public:
	static LightStorage &singleton() {
		static LightStorage instance;
		return instance;
	}

	LightStorage(LightStorage const &) = delete;
	void operator=(LightStorage const &) = delete;

private:
	LightStorage() {}

public:
};

#endif // !LIGHT_STORAGE_H
