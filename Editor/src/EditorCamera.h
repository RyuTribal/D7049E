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

		const float c_ZoomSpeed = 0.1;
		float m_Speed = 1000.0f;
		glm::vec3 m_Velocity = { 0.f, 0.f, 0.f };
		float m_AirFriction = 500.f;
		float m_Sensitivity = 1.f;
		const float m_SmoothingFactor = 0.9f;
		std::map<int, bool> m_KeyStates;
		bool m_FirstClick = true;
		glm::vec2 m_DeltaMouseOrientation = { 0.f, 0.f };
		glm::vec2 m_CurrentMouseOrientation = { 0.f, 0.f };
	};
}