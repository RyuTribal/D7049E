#pragma once

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

		glm::vec3& GetDirection() { return m_Direction; }
		void SetDirection(glm::vec3& direction) { m_Direction = direction; }

		glm::vec3& GetColor() { return m_Color; }
		void SetColor(glm::vec3& color) { m_Color = color; }

		float GetIntensity() { return m_Intensity; }
		void SetIntensity(float intensity) { m_Intensity = intensity; }

		bool IsCastingShadows() { return m_CastShadows; }
		void CastShadows(bool cast) { m_CastShadows = cast; }

	private:
		glm::vec3 m_Direction = { 0.0f, 0.0f, 0.0f };
		glm::vec3 m_Color = { 1.f, 1.f, 1.f };
		float m_Intensity = 1.f;
		bool m_CastShadows = false;
	};
}
