#include "EditorLayer.h"
#include "Panels/Viewport.h"

namespace Editor {
	void EditorLayer::OnAttach()
	{
		auto [scene, camera_entity_handle] = Engine::Scene::CreateScene("Editor Scene");
		m_Camera = Engine::CreateRef<EditorCamera>(scene->GetCurrentCamera(), camera_entity_handle);
		m_Scene = scene;
		Engine::Renderer::Get()->SetBackgroundColor(0, 0, 0);

		Cuboid cube{ 1.f, 1.f, 1.f };
		Engine::Ref<Engine::Material> cube_material = Engine::CreateRef<Silver>();

		Engine::EntityHandle* cube_entity_handle = m_Scene->CreateEntity("Cube", nullptr);
		Engine::Entity* cube_entity = m_Scene->GetEntity(cube_entity_handle);
		Engine::TransformComponent cube_transform{ glm::vec3(0.f, 0.f, 0.f) };
		cube_transform.local_transform.scale = glm::vec3(0.5f, 0.5f, 0.5f);

		cube_entity->AddComponent<Engine::MeshComponent>(cube.GetMesh());
		cube_entity->AddComponent<Engine::MaterialComponent>(cube_material);
		cube_entity->AddComponent<Engine::TransformComponent>(cube_transform);

		entities.push_back(cube_entity_handle); // Prevent garbage collection


		Engine::EntityHandle* light_entity_handle = m_Scene->CreateEntity("Sun", nullptr);
		Engine::Entity* light_entity = m_Scene->GetEntity(light_entity_handle);
		Engine::Ref<Engine::PointLight> light = Engine::CreateRef<Engine::PointLight>(); // defaults to white
		Engine::TransformComponent new_transform(glm::vec3(0.5f, 0.7f, 0.f));
		Engine::PointLightComponent new_light(light);

		light_entity->AddComponent<Engine::TransformComponent>(new_transform);
		light_entity->AddComponent<Engine::PointLightComponent>(new_light);

		entities.push_back(light_entity_handle);
	}

	void EditorLayer::OnUpdate(float delta_time)
	{
		Engine::Camera* curr_camera = m_Scene->GetCurrentCamera();
		Engine::Renderer::Get()->BeginFrame(curr_camera);
		m_Camera->Update(delta_time);
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

	bool EditorLayer::OnKeyPress(Engine::KeyPressedEvent& event)
	{
		m_Camera->UpdateKeyState(event.GetKeyCode(), true);
		return true;
	}

	bool EditorLayer::OnKeyRelease(Engine::KeyReleasedEvent& event)
	{
		m_Camera->UpdateKeyState(event.GetKeyCode(), false);
		return true;
	}

	bool EditorLayer::OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& event)
	{
		m_Camera->UpdateKeyState(MOUSE_BUTTON_RIGHT, false);
		return true;
	}

	bool EditorLayer::OnMouseMoved(Engine::MouseMovedEvent& event)
	{
		m_Camera->PanCamera();
		return true;
	}

	bool EditorLayer::OnMouseButtonPressed(Engine::MouseButtonPressedEvent& event)
	{
		m_Camera->UpdateKeyState(MOUSE_BUTTON_RIGHT, true);
		return true;
	}
}