#pragma once
#include <glad/gl.h>

namespace Engine
{
	class Shader
	{
	public:
		Shader(const std::string& path, GLuint type);
		~Shader();

		void CreateShader(GLuint& shader_ref, const std::string& path, GLuint type);

		GLuint Handle() { return m_ShaderHandle; }

	private:
		GLuint m_ShaderHandle;
	};
}