#include "pch.h"
#include "Camera.h"
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

	Camera::Camera() : m_Type(CameraType::PERSPECTIVE)
	{
		SetPerspective();
	}

	Camera::Camera(CameraType type) : m_Type(type)
	{
		if (m_Type == CameraType::ORTHOGRAPHIC)
		{
			SetOrthographic();
		}
		else if (m_Type == CameraType::PERSPECTIVE)
		{
			SetPerspective();
		}
	}
	void Camera::SetOrthographicSize(float size)
	{
		m_OrthographicSize = size;
		if (m_Type == CameraType::ORTHOGRAPHIC)
		{
			SetOrthographic();
		}
	}
	void Camera::SetClippingRange(float near_value, float far_value)
	{
		m_Near = near_value;
		m_Far = far_value;
		if (m_Type == CameraType::PERSPECTIVE)
		{
			SetPerspective();
		}
		else if (m_Type == CameraType::ORTHOGRAPHIC)
		{
			SetOrthographic();
		}
	}
	void Camera::SetFovy(float fovy)
	{
		m_PerspectiveFOVY = glm::radians(fovy);

		if (m_Type == CameraType::PERSPECTIVE)
		{
			SetPerspective();
		}
	}
	void Camera::SetAspectRatio(float ratio)
	{
		m_AspectRatio = ratio;

		if (m_Type == CameraType::PERSPECTIVE)
		{
			SetPerspective();
		}
		else if (m_Type == CameraType::ORTHOGRAPHIC)
		{
			SetOrthographic();
		}
	}
	std::pair<float, float> Camera::GetDeltaOrientation(const glm::vec2& delta, float rotation_speed, bool inverse_controls)
	{
		float yawSign = GetUpDirection().y < 0 ? -1.0f : 1.0f;
		float deltaYaw = yawSign * delta.x * rotation_speed;
		float deltaPitch = delta.y * rotation_speed;

		if (!inverse_controls)
		{
			deltaYaw *= -1;
			deltaPitch *= -1;
		}

		return { deltaPitch, deltaYaw };
	}
	void Camera::ChangeCameraType(CameraType type)
	{
		if (type != m_Type)
		{
			m_Type = type;
			m_Type == CameraType::ORTHOGRAPHIC ? SetOrthographic() : SetPerspective();
		}
	}
	std::vector<glm::vec4>& Camera::GetFrustumCornersWorldSpace()
	{
		return m_FurstumCorners;
	}
	void Camera::RecalculateViewMatrix()
	{
		m_Position = CalculatePosition();

		glm::quat orientation = GetOrientation();

		m_ViewMatrix = glm::translate(glm::mat4(1.0f), m_Position) * glm::toMat4(orientation);
		m_ViewMatrix = glm::inverse(m_ViewMatrix);
		SetFrustumCornersWorldSpace();
	}
	void Camera::SetOrthographic()
	{
		float orthoLeft = -m_OrthographicSize * m_AspectRatio * 0.5f;
		float orthoRight = m_OrthographicSize * m_AspectRatio * 0.5f;
		float orthoBottom = -m_OrthographicSize * 0.5f;
		float orthoTop = m_OrthographicSize * 0.5f;
		m_ProjectionMatrix = glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, m_Near, m_Far);
	}
	void Camera::SetPerspective()
	{
		m_ProjectionMatrix = glm::perspective(m_PerspectiveFOVY, m_AspectRatio, m_Near, m_Far);
		SetFrustumCornersWorldSpace();
	}

	void Camera::SetFrustumCornersWorldSpace()
	{
		const auto inv = glm::inverse(m_ProjectionMatrix * m_ViewMatrix);

		m_FurstumCorners.clear();
		for (unsigned int x = 0; x < 2; ++x)
		{
			for (unsigned int y = 0; y < 2; ++y)
			{
				for (unsigned int z = 0; z < 2; ++z)
				{
					const glm::vec4 pt =
						inv * glm::vec4(
							2.0f * x - 1.0f,
							2.0f * y - 1.0f,
							2.0f * z - 1.0f,
							1.0f);
					m_FurstumCorners.push_back(pt / pt.w);
				}
			}
		}
	}

	void Camera::RotateAroundFocalPoint(const glm::vec2& delta, float rotation_speed, bool inverse_controls)
	{
		float sign = inverse_controls ? -1.0f : 1.0f;

		float deltaYaw = delta.x * rotation_speed * sign;
		float deltaPitch = delta.y * rotation_speed * sign;

		m_Yaw += deltaYaw;
		m_Pitch += deltaPitch;

		glm::quat pitchQuat = glm::angleAxis(deltaPitch, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat yawQuat = glm::angleAxis(deltaYaw, glm::vec3(0.0f, 1.0f, 0.0f));

		m_Orientation = glm::normalize(yawQuat * m_Orientation * pitchQuat);

		CalculatePosition();
	}

	void Camera::SetRotationAroundFocalPoint(const glm::vec2& rotation)
	{

		m_Yaw = rotation.y;
		m_Pitch = rotation.x;

		glm::quat pitchQuat = glm::angleAxis(m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat yawQuat = glm::angleAxis(m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f));

		m_Orientation = glm::normalize(yawQuat * pitchQuat);

		CalculatePosition();
	}

	void Camera::Rotate(const glm::vec2& delta, float rotation_speed, bool inverse_controls)
	{
		float sign = inverse_controls ? 1.0f : -1.0f;
		m_Yaw += delta.x * rotation_speed * sign;
		m_Pitch += delta.y * rotation_speed * sign;

		float pitchLimit = glm::radians(89.0f); // Prevent flipping over
		m_Pitch = glm::clamp(m_Pitch, -pitchLimit, pitchLimit);

		glm::quat pitchQuat = glm::angleAxis(m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat yawQuat = glm::angleAxis(m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f));

		m_Orientation = glm::normalize(yawQuat * pitchQuat);
	}

	void Camera::SetRotation(const glm::vec2& rotation)
	{
		m_Yaw = rotation.x;
		m_Pitch = rotation.y;

		float pitchLimit = glm::radians(89.0f);
		m_Pitch = glm::clamp(m_Pitch, -pitchLimit, pitchLimit);

		glm::quat pitchQuat = glm::angleAxis(m_Pitch, glm::vec3(1.0f, 0.0f, 0.0f));
		glm::quat yawQuat = glm::angleAxis(m_Yaw, glm::vec3(0.0f, 1.0f, 0.0f));

		m_Orientation = glm::normalize(yawQuat * pitchQuat);
	}

	void Camera::LookAt(glm::vec3& center)
	{
		glm::vec3 direction = glm::normalize(center - m_Position);
		m_Orientation = glm::quatLookAt(direction, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	/*void Camera::LookAt(glm::vec3& center)
	{
		glm::vec3 direction = glm::normalize(center - CalculatePosition());
		m_Pitch = asin(direction.y);
		m_Yaw = atan2(direction.x, -direction.z);
	}*/

	glm::vec3 Camera::GetUpDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 1.0f, 0.0f));
	}

	glm::vec3 Camera::GetRightDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(1.0f, 0.0f, 0.0f));
	}

	glm::vec3 Camera::GetForwardDirection() const
	{
		return glm::rotate(GetOrientation(), glm::vec3(0.0f, 0.0f, -1.0f));
	}

	glm::vec3 Camera::CalculatePosition() const
	{
		return m_FocalPoint - GetForwardDirection() * m_Distance;
	}

	glm::quat Camera::GetOrientation() const
	{
		return m_Orientation;
	}
}
