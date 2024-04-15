#include "pch.h"
#include "UniformBuffer.h"
#include <glad/gl.h>

namespace Engine {
	UniformBuffer::UniformBuffer(uint32_t size, uint32_t binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_UNIFORM_BUFFER, binding, m_RendererID);
	}

	UniformBuffer::~UniformBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}


	void UniformBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}


	ShaderStorageBuffer::ShaderStorageBuffer(uint32_t size, uint32_t binding) : m_Binding(binding)
	{
		glCreateBuffers(1, &m_RendererID);
		glNamedBufferData(m_RendererID, size, nullptr, GL_DYNAMIC_DRAW);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, binding, m_RendererID);
	}

	ShaderStorageBuffer::~ShaderStorageBuffer()
	{
		glDeleteBuffers(1, &m_RendererID);
	}

	void ShaderStorageBuffer::SetData(const void* data, uint32_t size, uint32_t offset)
	{
		glNamedBufferSubData(m_RendererID, offset, size, data);
	}

	void ShaderStorageBuffer::Bind() const
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_Binding, m_RendererID);
	}


	void ShaderStorageBuffer::Unbind() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void ShaderStorageBuffer::Unmap() const
	{
		glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
	}

	void* ShaderStorageBuffer::Map() const
	{
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_RendererID);
		return glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_WRITE);
	}

}
