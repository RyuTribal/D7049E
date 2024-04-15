#pragma once

#include <Engine.h>
#include <map>

using namespace Engine;

namespace Editor {
	class EditorCamera {
	public:
		EditorCamera(Camera* camera, EntityHandle* handle, Ref<Scene> context) : m_Camera(camera), m_EntityHandle(handle), m_Context(context) {
			m_CurrentMouseOrientation.x = Input::GetMouseX();
			m_CurrentMouseOrientation.y = Input::GetMouseY();
			m_Camera->Zoom(m_InitialZoomFactor);
			m_Camera->RotateWithVector(m_InitialRotation);
			m_Context->GetEntity(handle)->GetComponent<TransformComponent>()->local_transform.translation = m_InitialPosition;
		}
		~EditorCamera() = default;

		Camera* GetCamera() { return m_Camera; }
		EntityHandle* GetHandle() { return m_EntityHandle; }

		void Update(float delta_time);
		void PanCamera();
		void Zoom(float offset);
		void UpdateKeyState(int keyCode, bool isPressed);

	private:
		void BuildVelocityVector(float delta_time);
		bool HasMovement();
		void UpdateMovement(float delta_time);
		void ApplyFriction(float delta_time);
	private:
		EntityHandle* m_EntityHandle;
		Camera* m_Camera;
		Ref<Scene> m_Context;

		const float c_ZoomSpeed = 1.f;
		float m_Speed = 10000.0f;
		glm::vec3 m_Velocity = { 0.f, 0.f, 0.f };
		float m_AirFriction = 500.f;
		float m_Sensitivity = 1.f;
		const float m_SmoothingFactor = 0.9f;
		const float m_InitialZoomFactor = 10.f;
		std::map<int, bool> m_KeyStates;
		glm::vec3 m_InitialPosition = { 1.15f, 3.85f, 0.f };
		glm::vec3 m_InitialRotation = { -32.f, 30.f, 0.f };
		bool m_FirstClick = true;
		glm::vec2 m_DeltaMouseOrientation = { 0.f, 0.f };
		glm::vec2 m_CurrentMouseOrientation = { 0.f, 0.f };
	};
}
