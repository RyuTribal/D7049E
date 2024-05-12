#pragma once
#include "ShaderProgram.h"
#include <variant>

namespace Engine {
	template<typename T>
	inline T GetDefaultValue();

	template<>
	inline float GetDefaultValue<float>() { return 0.0f; }

	template<>
	inline int GetDefaultValue<int>() { return 0; }

	template<>
	inline uint32_t GetDefaultValue<uint32_t>() { return 0u; }

	template<>
	inline glm::ivec2 GetDefaultValue<glm::ivec2>() { return glm::ivec2(0); }

	template<>
	inline glm::ivec3 GetDefaultValue<glm::ivec3>() { return glm::ivec3(0); }

	template<>
	inline glm::ivec4 GetDefaultValue<glm::ivec4>() { return glm::ivec4(0); }

	template<>
	inline glm::vec2 GetDefaultValue<glm::vec2>() { return glm::vec2(0.0f); }

	template<>
	inline glm::vec3 GetDefaultValue<glm::vec3>() { return glm::vec3(0.0f); }

	template<>
	inline glm::vec4 GetDefaultValue<glm::vec4>() { return glm::vec4(0.0f); }

	template<>
	inline glm::mat3 GetDefaultValue<glm::mat3>() { return glm::mat3(1.0f); }

	template<>
	inline glm::mat4 GetDefaultValue<glm::mat4>() { return glm::mat4(1.0f); }



	enum TextureSlots
	{
		Normal = 1,
		Roughness = 2,
		Metalness = 3,
		Albedo = 5,
		AO = 6,
		Emissive = 7,
		SpecularColor = 8,
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

		template<typename T>
		T GetUniformValue(const std::string& uniform_name) const
		{
			auto it = m_Uniforms.find(uniform_name);
			if (it != m_Uniforms.end())
			{
				const UniformValue& value = it->second;

				if (std::holds_alternative<T>(value))
				{
					return std::get<T>(value);
				}
			}

			return GetDefaultValue<T>();
		}

	protected:
		Ref<ShaderProgram> m_Program;
		std::unordered_map<uint32_t, Ref<Texture2D>> m_Textures;
		std::unordered_map<std::string, UniformValue> m_Uniforms;
	};
}
