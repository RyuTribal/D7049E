#include "pch.h"
#include "Material.h"
#include "Renderer.h"

namespace Engine {
	Material::Material(Ref<ShaderProgram> program) : m_Program(program)
	{
		
	}
	
	void Material::ApplyMaterial()
    {
		m_Program->Activate();
		for (auto& item : m_Textures)
		{
			item.second->Bind(item.first);
		}

    }
	void Material::Set(const std::string& name, float value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, int value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, uint32_t value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, bool value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, const glm::ivec2& value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, const glm::ivec3& value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, const glm::ivec4& value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, const glm::vec2& value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, const glm::vec3& value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, const glm::vec4& value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, const glm::mat3& value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, const glm::mat4& value)
	{
		m_Program->Set(name, value);
	}
	void Material::Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t slot)
	{
		m_Textures[slot] = texture;
		m_Program->Set(name, texture, slot);
	}
}
