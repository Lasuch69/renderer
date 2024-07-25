#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <vulkan/vulkan.h>

#include "shader.h"

#define CHECK_VK_RESULT(_expr, msg)                                                                                    \
	if (!(_expr)) {                                                                                                    \
		printf("%s\n", msg);                                                                                           \
	}

static VkShaderModule moduleCreate(VkDevice device, const uint32_t *pCode, size_t codeSize) {
	VkShaderModuleCreateInfo createInfo = {
		.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		.codeSize = codeSize,
		.pCode = pCode,
	};

	VkShaderModule shaderModule;
	CHECK_VK_RESULT(vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) == VK_SUCCESS,
			"Shader module creation failed!");

	return shaderModule;
}

void Shader::_setupGraphics(
		const uint32_t *vertexCode, size_t vertexCodeSize, const uint32_t *fragmentCode, size_t fragmentCodeSize) {
	m_vertexCode = (uint32_t *)malloc(vertexCodeSize);
	memcpy(m_vertexCode, vertexCode, vertexCodeSize);
	m_vertexCodeSize = vertexCodeSize;

	m_fragmentCode = (uint32_t *)malloc(fragmentCodeSize);
	memcpy(m_fragmentCode, fragmentCode, fragmentCodeSize);
	m_fragmentCodeSize = fragmentCodeSize;

	m_isCompute = false;
}

void Shader::_setupCompute(const uint32_t *computeCode, size_t computeCodeSize) {
	m_computeCode = (uint32_t *)malloc(computeCodeSize);
	memcpy(m_computeCode, computeCode, computeCodeSize);
	m_computeCodeSize = computeCodeSize;

	m_isCompute = true;
}

bool Shader::isCompute() const {
	return m_isCompute;
}

VkShaderModule Shader::vertexStage() const {
	return m_vertexModule;
}

VkShaderModule Shader::fragmentStage() const {
	return m_fragmentModule;
}

VkShaderModule Shader::computeStage() const {
	return m_computeModule;
}

void Shader::compile(VkDevice device) {
	if (!m_isCompute) {
		m_vertexModule = moduleCreate(device, m_vertexCode, m_vertexCodeSize);
		m_vertexCodeSize = 0;
		free(m_vertexCode);

		m_fragmentModule = moduleCreate(device, m_fragmentCode, m_fragmentCodeSize);
		m_fragmentCodeSize = 0;
		free(m_fragmentCode);
	} else {
		m_computeModule = moduleCreate(device, m_computeCode, m_computeCodeSize);
		m_computeCodeSize = 0;
		free(m_computeCode);
	}

	m_device = device;
}

Shader::~Shader() {
	if (!m_isCompute) {
		vkDestroyShaderModule(m_device, m_vertexModule, nullptr);
		vkDestroyShaderModule(m_device, m_fragmentModule, nullptr);
	} else {
		vkDestroyShaderModule(m_device, m_computeModule, nullptr);
	}
}
