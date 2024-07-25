#ifndef SHADER_H
#define SHADER_H

#include <cstddef>
#include <cstdint>

typedef struct VkDevice_T *VkDevice;
typedef struct VkShaderModule_T *VkShaderModule;

class Shader {
private:
	VkDevice m_device;

	uint32_t *m_vertexCode;
	size_t m_vertexCodeSize;

	uint32_t *m_fragmentCode;
	size_t m_fragmentCodeSize;

	uint32_t *m_computeCode;
	size_t m_computeCodeSize;

	VkShaderModule m_vertexModule;
	VkShaderModule m_fragmentModule;
	VkShaderModule m_computeModule;

	bool m_isCompute = false;

protected:
	void _setupGraphics(
			const uint32_t *vertexCode, size_t vertexCodeSize, const uint32_t *fragmentCode, size_t fragmentCodeSize);
	void _setupCompute(const uint32_t *computeCode, size_t computeCodeSize);

public:
	bool isCompute() const;

	VkShaderModule vertexStage() const;
	VkShaderModule fragmentStage() const;
	VkShaderModule computeStage() const;

	void compile(VkDevice device);

	~Shader();
};

#endif // !SHADER_H
