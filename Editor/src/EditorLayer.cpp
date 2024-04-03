#include "EditorLayer.h"
#include "Panels/Viewport.h"

namespace Editor {
	void EditorLayer::OnAttach()
	{
		auto [camera_entity, scene] = Engine::Scene::CreateScene("Editor Scene");
		m_Scene = scene;
		Engine::Renderer::Get()->SetBackgroundColor(0, 0, 0);


		checkerboard_tex = Engine::Texture2D::Create(ROOT_PATH + std::string("/assets/test.png"));

		Cuboid cube{ 1.f, 1.f, 1.f };
		Engine::Ref<Engine::Material> cube_material = Engine::CreateRef<Silver>();

		Engine::Ref<Engine::Entity> cube_entity = scene->CreateEntity("Cube", nullptr);
		Engine::TransformComponent cube_transform{ glm::vec3(0.f, 0.f, 0.f) };
		cube_transform.local_transform.scale = glm::vec3(0.5f, 0.5f, 0.5f);

		cube_entity->AddComponent<Engine::MeshComponent>(cube.GetMesh());
		cube_entity->AddComponent<Engine::MaterialComponent>(cube_material);
		cube_entity->AddComponent<Engine::TransformComponent>(cube_transform);

		entities[cube_entity->GetID()] = cube_entity; // Prevent garbage collection


		Engine::Ref<Engine::Entity> light_entity = scene->CreateEntity("Sun", nullptr);
		Engine::Ref<Engine::PointLight> light = Engine::CreateRef<Engine::PointLight>(); // defaults to white
		Engine::TransformComponent new_transform(glm::vec3(0.5f, 0.7f, 0.f));
		Engine::PointLightComponent new_light(light);

		light_entity->AddComponent<Engine::TransformComponent>(new_transform);
		light_entity->AddComponent<Engine::PointLightComponent>(new_light);

		entities[light_entity->GetID()] = light_entity;
	}

	void EditorLayer::OnUpdate(float delta_time)
	{
		Engine::Camera* curr_camera = m_Scene->GetCurrentCamera();
		Engine::Renderer::Get()->BeginFrame(curr_camera);
		smoothedDeltaTime = smoothingFactor * smoothedDeltaTime + (1.0f - smoothingFactor) * delta_time;
		UpdateMovement();
		if (!HasMovement()) {
			ApplyFriction();
		}
		m_Scene->UpdateScene();
		Engine::Renderer::Get()->EndFrame();
	}

	void EditorLayer::OnEvent(Engine::Event& event)
	{
		Engine::EventDispatcher dispatcher(event);

		dispatcher.Dispatch<Engine::KeyPressedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnKeyPress));
		dispatcher.Dispatch<Engine::KeyReleasedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnKeyRelease));
		dispatcher.Dispatch<Engine::MouseButtonPressedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnMouseButtonPressed));
		dispatcher.Dispatch<Engine::MouseButtonReleasedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnMouseButtonReleased));
		dispatcher.Dispatch<Engine::MouseMovedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnMouseMoved));
	}

	void EditorLayer::OnImGuiRender()
	{
		ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
		ImGui::Begin("Settings");
		if (ImGui::Button("Toggle Fullscreen")) {
			bool is_fullscreen = Engine::Application::Get().GetWindow().GetFullScreen();
			Engine::Application::Get().GetWindow().SetFullScreen(!is_fullscreen, Engine::BORDERLESS);
		}
		ImGui::End();
		EditorPanels::Viewport::Render();
		ImGui::ShowMetricsWindow();

	}

	void EditorLayer::SetDelta()
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

	bool EditorLayer::HasMovement()
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

	bool EditorLayer::OnKeyPress(Engine::KeyPressedEvent& event)
	{
		UpdateKeyState(event.GetKeyCode(), true);
		if (HasMovement()) {
			BuildVelocityVector();
		}
		return true;
	}

	bool EditorLayer::OnKeyRelease(Engine::KeyReleasedEvent& event)
	{
		UpdateKeyState(event.GetKeyCode(), false);
		return true;
	}

	bool EditorLayer::OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& event)
	{
		if (event.GetMouseButton() == MOUSE_BUTTON_RIGHT) {
			mouseIsPressed = false;
			Engine::Input::SetLockMouseMode(false);
		}
		return true;
	}

	bool EditorLayer::OnMouseMoved(Engine::MouseMovedEvent& event)
	{
		if (mouseIsPressed) {
			SetDelta();
		}
		return true;
	}

	void EditorLayer::BuildVelocityVector()
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

	bool EditorLayer::OnMouseButtonPressed(Engine::MouseButtonPressedEvent& event)
	{
		if (event.GetMouseButton() == MOUSE_BUTTON_RIGHT) {
			mouseIsPressed = true;
			firstClick = true;
			Engine::Input::SetLockMouseMode(true);
		}
		return true;
	}

	void EditorLayer::UpdateKeyState(int keyCode, bool isPressed)
	{
		keyStates[keyCode] = isPressed;
	}

	void EditorLayer::UpdateMovement()
	{
		auto camera = scene->GetCurrentCamera();

		if (mouseIsPressed) {
			camera->Rotate(deltaMouseOrientation * smoothedDeltaTime, sensitivity, false);
			deltaMouseOrientation = { 0.f, 0.f };
		}

		if (velocity != glm::vec3(0.0f)) {
			camera->Move(velocity * smoothedDeltaTime);
		}
	}

	void EditorLayer::ApplyFriction()
	{
		velocity *= 1.0 - air_friction;

		if (glm::length(velocity) < 0.01f) {
			velocity = glm::vec3(0.0f);
		}
	}
}