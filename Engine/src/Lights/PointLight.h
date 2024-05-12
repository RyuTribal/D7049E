#pragma once

namespace Engine {
	class PointLight {
	public:
		PointLight() = default;
		PointLight(const glm::vec3 color) { SetColor(color); };
		~PointLight() = default;

		glm::vec3& GetColor() { return m_Color; }

		void SetColor(const glm::vec3& color) { m_Color = color;}

		glm::vec3& GetPosition() { return m_Position; }
		float GetConstantAttenuation() { return m_ConstantAttenuation; }
		float GetLinearAttenuation() { return m_LinearAttenuation; }
		float GetQuadraticAttenuation() { return m_QuadraticAttenuation; }
		float GetIntensity() { return m_Intensity; }
		bool IsCastingShadows() { return m_CastShadows; }

		void SetPosition(const glm::vec3& position) { m_Position = position; }
		void SetConstantAttenuation(float attenuation) { m_ConstantAttenuation = attenuation; }
		void SetLinearAttenuation(float attenuation) { m_LinearAttenuation = attenuation; }
		void SetQuadraticAttenuation(float attenuation) { m_QuadraticAttenuation = attenuation; }
		void SetIntensity(float intensity) { m_Intensity = intensity; }
		void CastShadows(bool cast) { m_CastShadows = cast; }

	private:
		glm::vec3 m_Position = { 0.f, 0.f, 0.f };
		float m_ConstantAttenuation = 1.f;
		float m_LinearAttenuation = 0.07f;
		float m_QuadraticAttenuation = 0.18f;

		glm::vec3 m_Color = { 1.f, 1.f, 1.f };
		float m_Intensity = 1.f;

		bool m_CastShadows = false;
	};
}
