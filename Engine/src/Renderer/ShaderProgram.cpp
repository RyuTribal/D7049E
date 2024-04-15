#include "pch.h"
#include "ShaderProgram.h"
#include <glm/gtc/type_ptr.hpp>

namespace Engine {
	ShaderProgram::ShaderProgram(const std::string& path)
	{
        m_ShaderProgram = glCreateProgram();
        
        struct stat buffer;
        std::string full_path = path + ".vert";
        if (stat(full_path.c_str(), &buffer) == 0) {
            shaders.push_back(CreateScope<Shader>(full_path, GL_VERTEX_SHADER));
            glAttachShader(m_ShaderProgram, shaders[shaders.size()-1]->Handle());
        }
        full_path = path + ".frag";
        if (stat(full_path.c_str(), &buffer) == 0) {
            shaders.push_back(CreateScope<Shader>(full_path, GL_FRAGMENT_SHADER));
            glAttachShader(m_ShaderProgram, shaders[shaders.size() - 1]->Handle());
        }

        full_path = path + ".comp";
        if (stat(full_path.c_str(), &buffer) == 0) {
            shaders.push_back(CreateScope<Shader>(full_path, GL_COMPUTE_SHADER));
            glAttachShader(m_ShaderProgram, shaders[shaders.size() - 1]->Handle());
        }
        
        glLinkProgram(m_ShaderProgram);

        GLint success;
        GLchar infoLog[1024];
        glGetProgramiv(m_ShaderProgram, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(m_ShaderProgram, 512, NULL, infoLog);
            HVE_CORE_ERROR("Shader Linking Error: {0}, Perpetrator: {1}", infoLog, path);
            return;
        }
	}

    void ShaderProgram::UploadMat4FloatData(const std::string& name, const glm::mat4& matrix)
    {
        GLint location = glGetUniformLocation(m_ShaderProgram, name.c_str());
		std::string err_message = "Shader does not have a uniform " + name + "!";
		if (location != -1)
		{
			glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
		}

    }
    void ShaderProgram::UploadVec3FloatData(const std::string& name, const glm::vec3& vector)
    {
        GLint location = glGetUniformLocation(m_ShaderProgram, name.c_str());
		std::string err_message = "Shader does not have a uniform " + name + "!";
		if (location != -1)
		{
			glUniform3fv(location, 1, glm::value_ptr(vector));
		}
    }

    void ShaderProgram::UploadFloatData(const std::string& name, float data)
    {
        GLint location = glGetUniformLocation(m_ShaderProgram, name.c_str());
		std::string err_message = "Shader does not have a uniform " + name + "!";
		if (location != -1)
		{
			glUniform1f(location, data);
		}
    }

    void ShaderProgram::UploadVec2IntData(const std::string& name, const glm::ivec2& vector)
    {
        GLint location = glGetUniformLocation(m_ShaderProgram, name.c_str());
		std::string err_message = "Shader does not have a uniform " + name + "!";
		if (location != -1)
		{
			glUniform2iv(location, 1, glm::value_ptr(vector));
		}
    }

    void ShaderProgram::UploadIntData(const std::string& name, int data)
    {
        GLint location = glGetUniformLocation(m_ShaderProgram, name.c_str());
		std::string err_message = "Shader does not have a uniform " + name + "!";
		if (location != -1)
		{
			glUniform1i(location, data);
		}
    }

    ShaderProgram::~ShaderProgram()
    {
        glDeleteProgram(m_ShaderProgram);
    }
    void ShaderProgram::Activate()
    {
        glUseProgram(m_ShaderProgram);
    }
}
