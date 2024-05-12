#pragma once
#include "Shader.h"
#include "Texture.h"

namespace Engine {
	class ShaderProgram {
	public:
		ShaderProgram(const std::string& path);
		~ShaderProgram();
		GLuint GetProgram() { return m_ShaderProgram; }
		static Ref<ShaderProgram> Create(const std::string& path)
		{
			return CreateRef<ShaderProgram>(path);
		}


		void Activate();
		void Deactivate();
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

	private:
		GLuint m_ShaderProgram;
		std::vector<Scope<Shader>> shaders{};
	};

	class ShaderLibrary
	{
	public:
		ShaderLibrary();
		~ShaderLibrary();

		void Load(std::string_view name, const std::string& path);

		const Ref<ShaderProgram>& Get(const std::string& name) const;
		const Ref<ShaderProgram>& GetByShaderID(uint32_t id) const;
		size_t GetSize() const { return m_Shaders.size(); }

		std::unordered_map<std::string, Ref<ShaderProgram>>& GetShaders() { return m_Shaders; }
		const std::unordered_map<std::string, Ref<ShaderProgram>>& GetShaders() const { return m_Shaders; }
	private:
		std::unordered_map<std::string, Ref<ShaderProgram>> m_Shaders;
	};
}
