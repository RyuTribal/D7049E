#pragma once

#include <glm/glm.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Engine {

	enum CameraType
	{
		ORTHOGRAPHIC = 0,
		PERSPECTIVE = 1
	};

	class Camera
	{
	public:
		Camera();
		Camera(CameraType type);

		const glm::vec3& GetPosition() const { return m_FocalPoint; }
		float GetPitch() const { return m_Pitch; }
		float GetYaw() const { return m_Yaw; }
		float GetNear() { return m_Near; }
		float GetFar() { return m_Far; }
		float GetFOVY() { return glm::degrees(m_PerspectiveFOVY); }

		void SetPitch(float pitch) { m_Pitch = pitch; }
		void SetYaw(float yaw) { m_Yaw = yaw; }
		void SetNear(float near_value) { m_Near = near_value; }
		void SetFar(float far_value) { m_Far = far_value; }

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
		void SetClippingRange(float near_value, float far_value);
		void SetView(glm::mat4& view_matrix) { m_ViewMatrix = view_matrix; }
		void SetPosition(glm::vec3 translation) { m_FocalPoint = translation; }

		void RotateAroundFocalPoint(const glm::vec2& delta, float rotation_speed, bool inverse_controls);
		void SetRotationAroundFocalPoint(const glm::vec2& rotation);
		void Rotate(const glm::vec2& delta, float rotation_speed, bool inverse_controls);
		void SetRotation(const glm::vec2& rotation);

		void Move(const glm::vec3& velocity) { m_FocalPoint += velocity; };
		void LookAt(glm::vec3& center);
		/*
		* In degrees
		*/
		void SetFovy(float fovy);
		void SetAspectRatio(float ratio);
		void SetZoomDistance(float distance) { m_Distance = distance; }
		void Zoom(float offset) { m_Distance += offset; }

		std::pair<float, float> GetDeltaOrientation(const glm::vec2& delta, float rotation_speed, bool inverse_controls);

		void ChangeCameraType(CameraType type);
		void UpdateCamera() { RecalculateViewMatrix(); }
		glm::vec3 CalculatePosition() const;

		bool IsRotationLocked() { return b_LockedRotation; }
		void SetIsRotationLocked(bool locked) { b_LockedRotation = locked; }

		std::vector<glm::vec4>& GetFrustumCornersWorldSpace();

	private:
		void RecalculateViewMatrix();
		void SetOrthographic();
		void SetPerspective();
		void UpdateOrientation();
		void SetFrustumCornersWorldSpace();
	private:
		glm::mat4 m_ProjectionMatrix;
		glm::mat4 m_ViewMatrix{ 1.f };
		glm::mat4 m_ViewProjectionMatrix{ 1.f };
		std::vector<glm::vec4> m_FurstumCorners;
		
		glm::quat m_Orientation = { 0.f, 0.f, 0.f, 0.f };
		glm::vec3 m_Position = { 0.f, 0.f, 0.f };
		glm::vec3 m_FocalPoint = { 0.0f, 0.0f, 0.0f };
		float m_Yaw = 0.0f;
		float m_Pitch = 0.0f;
		float m_Distance = 1.0f;

		float m_OrthographicSize = 1.f;

		float m_Near = 0.1f;
		float m_Far = 500.f;

		float m_PerspectiveFOVY = glm::radians(45.f);

		float m_AspectRatio = 1280.f / 720.f;

		bool b_LockedRotation = false;


		CameraType m_Type;
	};


}
