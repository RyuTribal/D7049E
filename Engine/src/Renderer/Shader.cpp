#include "pch.h"
#include "Shader.h"


namespace Engine
{
	Shader::Shader(const std::string& path, GLuint type)
	{
        CreateShader(m_ShaderHandle ,path, type);
	}

    Shader::~Shader()
    {
        glDeleteShader(m_ShaderHandle);
    }

    void Shader::CreateShader(GLuint& shader_ref, const std::string& path, GLuint type) {
        std::ifstream shaderFile(path);
        HVE_CORE_ASSERT(shaderFile.is_open(), "Failed to open Shader file");

        std::stringstream buffer;
        buffer << shaderFile.rdbuf();
        std::string shaderCode = buffer.str();
        shaderFile.close();
        shader_ref = glCreateShader(type);
        const GLchar* shaderCodeCStr = shaderCode.c_str();
        glShaderSource(shader_ref, 1, &shaderCodeCStr, NULL);
        glCompileShader(shader_ref);

        GLint success;
        glGetShaderiv(shader_ref, GL_COMPILE_STATUS, &success);
        if (!success) {
            GLchar infoLog[1024];
            glGetShaderInfoLog(shader_ref, sizeof(infoLog), NULL, infoLog);
            glDeleteShader(shader_ref);
            shader_ref = 0;
            HVE_CORE_ERROR_TAG("Shader", "COMPILATION_ERROR of type: {} \n{} Perpetrator: {}", type, infoLog, path);
        }
    }
}