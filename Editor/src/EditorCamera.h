#pragma once

#include <Engine.h>
#include <map>

namespace Editor {
	class EditorCamera {
	public:
		EditorCamera(Engine::Camera* camera, Engine::EntityHandle* handle) : m_Camera(camera), m_EntityHandle(handle) {}
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
		float speed = 1000.0f;
		glm::vec3 velocity = { 0.f, 0.f, 0.f };
		float air_friction = 500.f;
		float sensitivity = 1.f;
		float smoothedDeltaTime = 0.0f;
		const float smoothingFactor = 0.9f;
		std::map<int, bool> keyStates;
		bool mouseIsPressed = false;
		bool firstClick = true;
		glm::vec2 deltaMouseOrientation = { 0.f, 0.f };
		glm::vec2 currentMouseOrientation = { 0.f, 0.f };
	};
}