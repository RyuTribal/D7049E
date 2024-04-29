#pragma once
#include "ShaderProgram.h"
#include <variant>

namespace Engine {
	enum TextureSlots
	{
		Normal = 1,
		Roughness = 2,
		Metalness = 3,
		Albedo = 5,
	};

	enum class MaterialUniforms
	{
		AlbedoColor,
		Emission,
		UseNormalMap,
		Roughness,
		Metalness
	};

	using UniformValue = std::variant<float, int, uint32_t, glm::ivec2, glm::ivec3, glm::ivec4, glm::vec2, glm::vec3, glm::vec4, glm::mat3, glm::mat4>;

	class Material
	{
	public:

		Material(Ref<ShaderProgram> program);

		void ApplyMaterial();

		Ref<ShaderProgram> GetProgram() { return m_Program; }
		void SetProgram(Ref<ShaderProgram> program) { m_Program = program; }

		void Set(const std::string& name, float value);
		void Set(const std::string& name, int value);
		void Set(const std::string& name, uint32_t value);
		void Set(const std::string& name, bool value);
		void Set(const std::string& name, const glm::ivec2& value);
		void Set(const std::string& name, const glm::ivec3& value);
		void Set(const std::string& name, const glm::ivec4& value);
		void Set(const std::string& name, const glm::vec2& value);
		void Set(const std::string& name, const glm::vec3& value);
		void Set(const std::string& name, const glm::vec4& value);
		void Set(const std::string& name, const glm::mat3& value);
		void Set(const std::string& name, const glm::mat4& value);
		void Set(const std::string& name, const Ref<Texture2D>& texture, uint32_t slot);

	protected:
		Ref<ShaderProgram> m_Program;
		std::unordered_map<uint32_t, Ref<Texture2D>> m_Textures;
		std::unordered_map<std::string, UniformValue> m_Uniforms;

	};
}
