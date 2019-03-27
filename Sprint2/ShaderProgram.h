#pragma once

#include "Renderer.h"

class ShaderProgram {
public:
	static ShaderProgram * Create( const char * shaderName );
	VkShaderModule GetVertexModule() const { return m_vertexShader; }
	VkShaderModule GetFragmentModule() const { return m_fragmentShader; }

private:
	VkShaderModule m_vertexShader = VK_NULL_HANDLE;
	VkShaderModule m_fragmentShader = VK_NULL_HANDLE;

private:
	ShaderProgram() = default;
};