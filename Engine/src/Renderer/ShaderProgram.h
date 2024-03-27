#pragma once
#include "Shader.h"

namespace Engine {
	class ShaderProgram {
	public:
		ShaderProgram(const std::string& path);
		~ShaderProgram();
		GLuint GetProgram() { return m_ShaderProgram; }

		void Activate();
		void UploadMat4FloatData(const std::string& name, const glm::mat4& matrix);
		void UploadVec3FloatData(const std::string& name, const glm::vec3& vector);
		void UploadFloatData(const std::string& name, float data);


		void UploadVec2IntData(const std::string& name, const glm::ivec2& vector);
		void UploadIntData(const std::string& name, int data);
	private:
		GLuint m_ShaderProgram;
		std::vector<Scope<Shader>> shaders{};
	};
}