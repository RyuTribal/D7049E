#pragma once

#include <glm/glm.hpp>

namespace Engine {

	enum CameraType {
		ORTHOGRAPHIC,
		PERSPECTIVE
	};

	class Camera {
	public:
		
		Camera(CameraType type);

		const glm::vec3& GetPosition() const { return m_FocalPoint; }
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		float GetFar() { return m_Far; }
		float GetFOVY() { return glm::degrees(m_PerspectiveFOVY); }

		void SetPitch(float pitch) { m_Pitch = pitch; }
		void SetYaw(float yaw) { m_Yaw = yaw; }
		void SetFar(float far) { m_Far = far; }
		 
		glm::vec3 GetUpDirection() const;
		glm::vec3 GetRightDirection() const;
		glm::vec3 GetForwardDirection() const;
		glm::quat GetOrientation() const;

		const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
		const glm::mat4& GetViewProjection() const { return m_ViewProjectionMatrix; }
		const glm::mat4& GetView() const { return m_ViewMatrix; }
		const CameraType& GetType() const { return m_Type; }
		float GetZoomDistance() { return m_Distance; }
		float GetOrthographicSize() { return m_OrthographicSize; }
		float GetAspectRatio() { return m_AspectRatio; }

		void SetOrthographicSize(float size);
		void SetClippingRange(float near, float far);
		void SetView(glm::mat4& view_matrix) { m_ViewMatrix = view_matrix; }
		void SetPosition(glm::vec3 translation) { m_FocalPoint = translation; }

		void Rotate(const glm::vec2& delta, float rotation_speed, bool inverse_controls);
		void RotateWithVector(glm::vec3& rotation);
		void Move(const glm::vec3& velocity) { m_FocalPoint += velocity; };
		void LookAt(glm::vec3& center);
		/*
		* In degrees
		*/
		void SetFovy(float fovy);
		void SetAspectRatio(float ratio);
		void SetZoomDistance(float distance) { m_Distance = distance; }

		std::pair<float, float> GetDeltaOrientation(const glm::vec2& delta, float rotation_speed, bool inverse_controls);

		void ChangeCameraType(CameraType type);
		void UpdateCamera() { RecalculateViewMatrix(); }
		glm::vec3 CalculatePosition() const;

	private:
		void RecalculateViewMatrix();
		void SetOrthographic();
		void SetPerspective();
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix{ 1.f };
		glm::mat4 m_ViewProjectionMatrix{ 1.f };

		glm::vec3 m_Position = { 0.f, 0.f, 0.f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };
		float m_Yaw = 0.0f;
		float m_Pitch = 0.0f;
		float m_Distance = 1.0f;

		float m_OrthographicSize = 1.f;

		float m_Near = 0.1f;
		float m_Far = 1000.f;

		float m_PerspectiveFOVY = glm::radians(45.f);

		float m_AspectRatio = 1280.f / 720.f;


		CameraType m_Type;
	};


}