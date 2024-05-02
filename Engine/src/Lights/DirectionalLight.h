#pragma once

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Engine {

	class DirectionalLight
	{
	public:
		DirectionalLight() = default;
		DirectionalLight(const glm::vec3 direction, const glm::vec3 color)
			: m_Direction(glm::normalize(direction)), m_Color(color)
		{
		}
		~DirectionalLight() = default;

		glm::quat& GetDirection() { return m_Direction; }
		void SetDirection(glm::quat& direction) { m_Direction = direction; }

		glm::vec3& GetColor() { return m_Color; }
		void SetColor(glm::vec3& color) { m_Color = color; }

		float GetIntensity() { return m_Intensity; }
		void SetIntensity(float intensity) { m_Intensity = intensity; }

		glm::vec3 DirectionToVec3()
		{
			glm::vec3 initialDir(0.0f, 0.0f, -1.0f);
			glm::vec3 lightDirection = glm::rotate(m_Direction, initialDir);
			return lightDirection;
		}

	private:
		glm::quat m_Direction = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec3 m_Color = { 1.f, 1.f, 1.f };
		float m_Intensity = 1.f;
	};
}
