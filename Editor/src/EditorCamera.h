#pragma once

#include <Engine.h>
#include <map>

namespace Editor {
	class EditorCamera {
	public:
		EditorCamera(Engine::Camera* camera, Engine::EntityHandle* handle) : m_Camera(camera), m_EntityHandle(handle) {
			m_CurrentMouseOrientation.x = Engine::Input::GetMouseX();
			m_CurrentMouseOrientation.y = Engine::Input::GetMouseY();
		}
		~EditorCamera() = default;

		Engine::Camera* GetCamera() { return m_Camera; }
		Engine::EntityHandle* GetHandle() { return m_EntityHandle; }

		void Update(float delta_time);
		void PanCamera();

		void UpdateKeyState(int keyCode, bool isPressed);

	private:
		void BuildVelocityVector(float delta_time);
		bool HasMovement();
		void UpdateMovement(float delta_time);
		void ApplyFriction(float delta_time);
	private:
		Engine::EntityHandle* m_EntityHandle;
		Engine::Camera* m_Camera;

		const float MAX_FRAME_TIME = 0.01667;
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