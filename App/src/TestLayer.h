#pragma once
#include <Engine.h>
#include <imgui/imgui.h>
#include "Primitives/Cuboid.h"
#include "Materials/Silver.h"
#include <map>


namespace TestApp {
	class TestLayer : public Engine::Layer {
	public:
		TestLayer(Engine::Ref<Engine::Scene> scene) : Layer("Test"), scene(scene) {

		}
		~TestLayer() = default;

		void OnAttach() override {
			Engine::Ref<Engine::Mesh> cube = Engine::CreateRef<Cuboid>(1.f, 1.f, 1.f);
			Engine::Ref<Engine::Material> cube_material = Engine::CreateRef<Silver>();

			Engine::Ref<Engine::Entity> cube_entity = scene->CreateEntity("Cube", nullptr);
			Engine::TransformComponent cube_transform{glm::vec3(0.f, 0.f, 0.f)};
			cube_transform.local_transform.scale = glm::vec3(0.5f, 0.5f, 0.5f);

			cube_entity->AddComponent<Engine::MeshComponent>(cube);
			cube_entity->AddComponent<Engine::MaterialComponent>(cube_material);
			cube_entity->AddComponent<Engine::TransformComponent>(cube_transform);

			entities[cube_entity->GetID()] = cube_entity; // Prevent garbage collection


			Engine::Ref<Engine::Entity> light_entity = scene->CreateEntity("Sun", nullptr);
			Engine::Ref<Engine::PointLight> light = Engine::CreateRef<Engine::PointLight>(); // defaults to white
			Engine::TransformComponent new_transform(glm::vec3(0.f, 0.7f, 0.f));
			Engine::PointLightComponent new_light(light);

			light_entity->AddComponent<Engine::TransformComponent>(new_transform);
			light_entity->AddComponent<Engine::PointLightComponent>(new_light);

			entities[light_entity->GetID()] = light_entity;
		}

		void OnUpdate(float delta_time) override {
			Engine::Camera *curr_camera = scene->GetCurrentCamera();
			Engine::Renderer::Get()->BeginFrame(curr_camera);
			smoothedDeltaTime = smoothingFactor * smoothedDeltaTime + (1.0f - smoothingFactor) * delta_time;
			UpdateMovement();
			if (!HasMovement()) {
				ApplyFriction();
			}
			scene->UpdateScene(delta_time);
			Engine::Renderer::Get()->EndFrame();
		}

		void OnEvent(Engine::Event& event) override
		{
			Engine::EventDispatcher dispatcher(event);

			dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_EVENT_FN(TestApp::TestLayer::OnKeyPress));
			dispatcher.Dispatch<Engine::KeyReleasedEvent>(BIND_EVENT_FN(TestApp::TestLayer::OnKeyRelease));
			dispatcher.Dispatch<Engine::MouseButtonPressedEvent>(BIND_EVENT_FN(TestApp::TestLayer::OnMouseButtonPressed));
			dispatcher.Dispatch<Engine::MouseButtonReleasedEvent>(BIND_EVENT_FN(TestApp::TestLayer::OnMouseButtonReleased));
			dispatcher.Dispatch<Engine::MouseMovedEvent>(BIND_EVENT_FN(TestApp::TestLayer::OnMouseMoved));
		}


		void SetDelta() {
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

		bool HasMovement()
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


		bool OnKeyPress(Engine::KeyPressedEvent& event)
		{
			UpdateKeyState(event.GetKeyCode(), true);
			if (HasMovement()) {
				BuildVelocityVector();
			}
			return true;
		}
		bool OnKeyRelease(Engine::KeyReleasedEvent& event)
		{
			UpdateKeyState(event.GetKeyCode(), false);
			return true;
		}
		bool OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& event)
		{
			if (event.GetMouseButton() == MOUSE_BUTTON_RIGHT) {
				mouseIsPressed = false;
				Engine::Input::SetLockMouseMode(false);
			}
			return true;
		}
		bool OnMouseMoved(Engine::MouseMovedEvent& event)
		{
			if (mouseIsPressed) {
				SetDelta();
			}
			return true;
		}

		void BuildVelocityVector()
		{
			auto camera = scene->GetCurrentCamera();
			velocity = glm::vec3(0.0f);

			if (keyStates[KEY_W]) {
				velocity += mouseIsPressed ? camera->GetForwardDirection() * speed : glm::vec3(0.0f, speed, 0.0f);
			}
			if (keyStates[KEY_S]) {
				velocity += mouseIsPressed ? -(camera->GetForwardDirection() * speed) : glm::vec3(0.0f, -speed, 0.0f);
			}
			if (keyStates[KEY_A]) {
				velocity += mouseIsPressed ? -(camera->GetRightDirection() * speed) : glm::vec3(-speed, 0.0f, 0.0f);
			}
			if (keyStates[KEY_D]) {
				velocity += mouseIsPressed ? camera->GetRightDirection() * speed : glm::vec3(speed, 0.0f, 0.0f);
			}
		}

		bool OnMouseButtonPressed(Engine::MouseButtonPressedEvent& event)
		{
			if (event.GetMouseButton() == MOUSE_BUTTON_RIGHT) {
				mouseIsPressed = true;
				firstClick = true;
				Engine::Input::SetLockMouseMode(true);
			}
			return true;
		}


		void UpdateKeyState(int keyCode, bool isPressed) {
			keyStates[keyCode] = isPressed;
		}

		void UpdateMovement() {
			auto camera = scene->GetCurrentCamera();

			if (mouseIsPressed) {
				camera->Rotate(deltaMouseOrientation * smoothedDeltaTime, sensitivity, false);
				deltaMouseOrientation = { 0.f, 0.f };
			}

			if (velocity != glm::vec3(0.0f)) {
				camera->Move(velocity * smoothedDeltaTime);
			}
		}
		void ApplyFriction() {
			velocity *= 1.0 - air_friction;

			if (glm::length(velocity) < 0.01f) {
				velocity = glm::vec3(0.0f);
			}
		}

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