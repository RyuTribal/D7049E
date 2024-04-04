#include "EditorCamera.h"

namespace Editor {

	void EditorCamera::Update(float delta_time)
	{
		UpdateMovement(delta_time);
		if (!HasMovement()) {
			ApplyFriction(delta_time);
		}
		else {
			BuildVelocityVector(delta_time);
		}
	}

	void EditorCamera::PanCamera()
	{
		if (firstClick) {
			currentMouseOrientation.x = Engine::Input::GetMouseX();
			currentMouseOrientation.y = Engine::Input::GetMouseY();
			firstClick = false;
		}

		deltaMouseOrientation = glm::vec2(0.f, 0.f);
		deltaMouseOrientation.x = Engine::Input::GetMouseX();
		deltaMouseOrientation.y = Engine::Input::GetMouseY();
		deltaMouseOrientation = currentMouseOrientation - deltaMouseOrientation;
		currentMouseOrientation = { 0.f, 0.f };
		currentMouseOrientation.x = Engine::Input::GetMouseX();
		currentMouseOrientation.y = Engine::Input::GetMouseY();
	}
	void EditorCamera::BuildVelocityVector(float delta_time)
	{
		velocity = glm::vec3(0.0f);
		if (keyStates[KEY_W]) {
			velocity += keyStates[MOUSE_BUTTON_RIGHT] ? m_Camera->GetForwardDirection() * speed * delta_time : m_Camera->GetUpDirection() * speed * delta_time;
		}
		if (keyStates[KEY_S]) {
			velocity += keyStates[MOUSE_BUTTON_RIGHT] ? -(m_Camera->GetForwardDirection() * speed * delta_time) : -(m_Camera->GetUpDirection() * speed * delta_time);
		}
		if (keyStates[KEY_A]) {
			velocity += keyStates[MOUSE_BUTTON_RIGHT] ? -(m_Camera->GetRightDirection() * speed * delta_time) : -(m_Camera->GetRightDirection() * speed * delta_time);
		}
		if (keyStates[KEY_D]) {
			velocity += keyStates[MOUSE_BUTTON_RIGHT] ? m_Camera->GetRightDirection() * speed * delta_time : m_Camera->GetRightDirection() * speed * delta_time;
		}
	}
	bool EditorCamera::HasMovement()
	{
		if (keyStates[KEY_W]) {
			return true;
		}
		else if (keyStates[KEY_S]) {
			return true;
		}
		else if (keyStates[KEY_A]) {
			return true;
		}
		else if (keyStates[KEY_D]) {
			return true;
		}
		return false;
	}
	void EditorCamera::UpdateMovement(float delta_time)
	{

		if (keyStates[MOUSE_BUTTON_RIGHT]) {
			m_Camera->Rotate(deltaMouseOrientation * delta_time, sensitivity, false);
			deltaMouseOrientation = { 0.f, 0.f };
		}

		if (velocity != glm::vec3(0.0f)) {
			m_Camera->Move(velocity * delta_time);
		}
	}
	void EditorCamera::ApplyFriction(float delta_time)
	{
		float frictionEffect = 1.0f - (air_friction * delta_time);

		velocity *= frictionEffect;

		if (glm::length(velocity) < 0.01f) {
			velocity = glm::vec3(0.0f);
		}
	}
	void EditorCamera::UpdateKeyState(int keyCode, bool isPressed)
	{
		if (keyCode == MOUSE_BUTTON_RIGHT && isPressed) {
			Engine::Input::SetLockMouseMode(true);
		}
		else if(keyCode == MOUSE_BUTTON_RIGHT && !isPressed) {
			Engine::Input::SetLockMouseMode(false);
		}
		keyStates[keyCode] = isPressed;
	}
}
