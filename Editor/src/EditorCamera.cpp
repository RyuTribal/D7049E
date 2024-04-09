#include "EditorCamera.h"

namespace Editor {

	void EditorCamera::Update(float delta_time)
	{
		UpdateMovement(delta_time);
		if (m_KeyStates[MOUSE_BUTTON_RIGHT]) {
			Engine::Input::SetLockMouseMode(true);
		}
		else {
			Engine::Input::SetLockMouseMode(false);
		}
		if (!HasMovement()) {
			ApplyFriction(delta_time);
		}
		else {
			BuildVelocityVector(delta_time);
		}
	}

	void EditorCamera::PanCamera()
	{
		if (m_FirstClick) {
			m_CurrentMouseOrientation.x = Engine::Input::GetMouseX();
			m_CurrentMouseOrientation.y = Engine::Input::GetMouseY();
			m_FirstClick = false;
		}
		m_DeltaMouseOrientation = glm::vec2(0.f, 0.f);
		m_DeltaMouseOrientation.x = Engine::Input::GetMouseX();
		m_DeltaMouseOrientation.y = Engine::Input::GetMouseY();
		m_DeltaMouseOrientation = m_CurrentMouseOrientation - m_DeltaMouseOrientation;
		m_CurrentMouseOrientation = { 0.f, 0.f };
		m_CurrentMouseOrientation.x = Engine::Input::GetMouseX();
		m_CurrentMouseOrientation.y = Engine::Input::GetMouseY();
	}
	void EditorCamera::Zoom(float offset)
	{
		m_Camera->Zoom(-offset * c_ZoomSpeed);
	}
	void EditorCamera::BuildVelocityVector(float delta_time)
	{
		m_Velocity = glm::vec3(0.0f);
		if (m_KeyStates[KEY_W]) {
			m_Velocity += m_KeyStates[MOUSE_BUTTON_RIGHT] ? m_Camera->GetForwardDirection() * m_Speed * delta_time : m_Camera->GetUpDirection() * m_Speed * delta_time;
		}
		if (m_KeyStates[KEY_S]) {
			m_Velocity += m_KeyStates[MOUSE_BUTTON_RIGHT] ? -(m_Camera->GetForwardDirection() * m_Speed * delta_time) : -(m_Camera->GetUpDirection() * m_Speed * delta_time);
		}
		if (m_KeyStates[KEY_A]) {
			m_Velocity += -(m_Camera->GetRightDirection() * m_Speed * delta_time);
		}
		if (m_KeyStates[KEY_D]) {
			m_Velocity += m_Camera->GetRightDirection() * m_Speed * delta_time;
		}
	}
	bool EditorCamera::HasMovement()
	{
		if (m_KeyStates[KEY_W]) {
			return true;
		}
		else if (m_KeyStates[KEY_S]) {
			return true;
		}
		else if (m_KeyStates[KEY_A]) {
			return true;
		}
		else if (m_KeyStates[KEY_D]) {
			return true;
		}
		return false;
	}
	void EditorCamera::UpdateMovement(float delta_time)
	{
		auto transform_comp = m_Context->GetEntity(m_EntityHandle)->GetComponent<TransformComponent>();

		if (m_KeyStates[MOUSE_BUTTON_RIGHT]) {
			auto camera_rotation = m_Camera->GetDeltaOrientation(m_DeltaMouseOrientation * delta_time, m_Sensitivity, true);
			transform_comp->local_transform.rotation.x += camera_rotation.first;
			transform_comp->local_transform.rotation.y += camera_rotation.second;
			m_DeltaMouseOrientation = { 0.f, 0.f };
		}

		if (m_Velocity != glm::vec3(0.0f)) {
			transform_comp->local_transform.translation += m_Velocity * delta_time;
		}
	}
	void EditorCamera::ApplyFriction(float delta_time)
	{
		float frictionEffect = 1.0f - (m_AirFriction * delta_time);

		m_Velocity *= frictionEffect;

		if (glm::length(m_Velocity) < 0.01f) {
			m_Velocity = glm::vec3(0.0f);
		}
	}
	void EditorCamera::UpdateKeyState(int keyCode, bool isPressed)
	{
		if (keyCode == MOUSE_BUTTON_RIGHT && isPressed) {
			m_FirstClick = true;
		}
		m_KeyStates[keyCode] = isPressed;
	}
}
