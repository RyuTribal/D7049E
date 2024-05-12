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

		full_path = path + ".geo";
		if (stat(full_path.c_str(), &buffer) == 0)
		{
			shaders.push_back(CreateScope<Shader>(full_path, GL_GEOMETRY_SHADER));
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

    ShaderProgram::~ShaderProgram()
    {
        glDeleteProgram(m_ShaderProgram);
    }
    void ShaderProgram::Activate()
    {
        glUseProgram(m_ShaderProgram);
    }

	void ShaderProgram::Deactivate()
	{
		glUseProgram(0);
	}

	void ShaderProgram::Set(const std::string& name, float value)
	{
		Activate();
		glUniform1f(glGetUniformLocation(m_ShaderProgram, name.c_str()), value);
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, int value)
	{
		Activate();
		glUniform1i(glGetUniformLocation(m_ShaderProgram, name.c_str()), value);
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, uint32_t value)
	{
		Activate();
		glUniform1ui(glGetUniformLocation(m_ShaderProgram, name.c_str()), value);
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, bool value)
	{
		Activate();
		glUniform1i(glGetUniformLocation(m_ShaderProgram, name.c_str()), (int)value);
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, const glm::ivec2& value)
	{
		Activate();
		glUniform2iv(glGetUniformLocation(m_ShaderProgram, name.c_str()), 1, glm::value_ptr(value));
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, const glm::ivec3& value)
	{
		Activate();
		glUniform3iv(glGetUniformLocation(m_ShaderProgram, name.c_str()), 1, glm::value_ptr(value));
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, const glm::ivec4& value)
	{
		Activate();
		glUniform4iv(glGetUniformLocation(m_ShaderProgram, name.c_str()), 1, glm::value_ptr(value));
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, const glm::vec2& value)
	{
		Activate();
		glUniform2fv(glGetUniformLocation(m_ShaderProgram, name.c_str()), 1, glm::value_ptr(value));
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, const glm::vec3& value)
	{
		Activate();
		glUniform3fv(glGetUniformLocation(m_ShaderProgram, name.c_str()), 1, glm::value_ptr(value));
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, const glm::vec4& value)
	{
		Activate();
		glUniform4fv(glGetUniformLocation(m_ShaderProgram, name.c_str()), 1, glm::value_ptr(value));
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, const glm::mat3& value)
	{
		Activate();
		glUniformMatrix3fv(glGetUniformLocation(m_ShaderProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, const glm::mat4& value)
	{
		Activate();
		glUniformMatrix4fv(glGetUniformLocation(m_ShaderProgram, name.c_str()), 1, GL_FALSE, glm::value_ptr(value));
		Deactivate();
	}

	void ShaderProgram::Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t slot)
	{
		Activate();
		texture->Bind(slot);
		glUniform1i(glGetUniformLocation(m_ShaderProgram, name.c_str()), slot);
		Deactivate();
	}

	ShaderLibrary::ShaderLibrary()
	{
	}

	ShaderLibrary::~ShaderLibrary()
	{
	}

	void ShaderLibrary::Load(std::string_view name, const std::string& path)
	{
		HVE_CORE_ASSERT(m_Shaders.find(std::string(name)) == m_Shaders.end());
		m_Shaders[std::string(name)] = ShaderProgram::Create(path);
	}

	const Ref<ShaderProgram>& ShaderLibrary::Get(const std::string& name) const
	{
		HVE_CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end());
		return m_Shaders.at(name);
	}
	const Ref<ShaderProgram>& ShaderLibrary::GetByShaderID(uint32_t id) const
	{
		for (auto [name, shader] : m_Shaders)
		{
			if (shader->GetProgram() == id)
			{
				return shader;
			}
		}
		HVE_CORE_ASSERT(false);
	}
}
