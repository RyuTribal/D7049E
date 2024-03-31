#pragma once
#include <Engine.h>
#include <imgui/imgui.h>
#include "Primitives/Cuboid.h"
#include "Materials/Silver.h"
#include <map>


namespace Editor {
	class EditorLayer : public Engine::Layer {
	public:
		EditorLayer(Engine::Ref<Engine::Scene> scene) : Layer("Test"), scene(scene) {

		}
		~EditorLayer() = default;

		void OnAttach() override;
		void OnUpdate(float delta_time) override;
		void OnEvent(Engine::Event& event) override;
		void OnImGuiRender() override;
		void SetDelta();
		bool HasMovement();
		bool OnKeyPress(Engine::KeyPressedEvent& event);
		bool OnKeyRelease(Engine::KeyReleasedEvent& event);
		bool OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& event);
		bool OnMouseMoved(Engine::MouseMovedEvent& event);
		void BuildVelocityVector();
		bool OnMouseButtonPressed(Engine::MouseButtonPressedEvent& event);
		void UpdateKeyState(int keyCode, bool isPressed);
		void UpdateMovement();
		void ApplyFriction();

	private:
		Engine::Ref<Engine::Scene> scene;
		std::unordered_map <Engine::UUID, Engine::Ref<Engine::Entity>> entities{}; //To make sure the entities are not garbage collected

		const float MAX_FRAME_TIME = 0.01667;

		float speed = 1.0f;

		glm::vec3 velocity = { 0.f, 0.f, 0.f };

		float air_friction = 0.1f;

		float sensitivity = 0.1f;

		float smoothedDeltaTime = 0.0f;
		const float smoothingFactor = 0.9f;

		std::map<int, bool> keyStates;
		bool mouseIsPressed = false;
		bool firstClick = true;

		glm::vec2 deltaMouseOrientation = { 0.f, 0.f };
		glm::vec2 currentMouseOrientation = { 0.f, 0.f };
	};
}