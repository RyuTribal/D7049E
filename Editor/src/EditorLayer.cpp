#include "EditorLayer.h"
#include "Panels/Viewport.h"
#include <imgui/imgui_internal.h>

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

		entities.push_back(cube_entity_handle);

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
		if (EditorPanels::Viewport::IsFocused()) {
			m_Camera->Update(delta_time);
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
		static bool dockspaceOpen = true;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
		static bool opt_fullscreen_persistant = true;
		bool opt_fullscreen = opt_fullscreen_persistant;


		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

		if (!b_EditDockspace) {
			dockspace_flags |= ImGuiDockNodeFlags_NoUndocking;
		}

		if (opt_fullscreen)
		{
			ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->Pos);
			ImGui::SetNextWindowSize(viewport->Size);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}


		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		ImGuiIO& io = ImGui::GetIO();
		ImGuiStyle& style = ImGui::GetStyle();
		float minWinSizeX = style.WindowMinSize.x;
		style.WindowMinSize.x = 370.0f;
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		style.WindowMinSize.x = minWinSizeX;

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("Open Project...", "Ctrl+O"))

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
					Engine::Application::Get().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				// Dont know yet, this is just more here for now
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}


		ImGui::Begin("Scene Hierarchy");
		
		ImGui::End();

		ImGui::Begin("Properties");

		ImGui::End();

		ImGui::Begin("Content Browser");

		ImGui::End();

		EditorPanels::Viewport::Render();



		ImGui::ShowMetricsWindow();

		ImGui::Begin("Settings");

		ImGui::End();

		ImGui::End();

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
		m_Camera->UpdateKeyState(event.GetMouseButton(), false);
		return true;
	}

	bool EditorLayer::OnMouseMoved(Engine::MouseMovedEvent& event)
	{
		m_Camera->PanCamera();
		return true;
	}

	bool EditorLayer::OnMouseButtonPressed(Engine::MouseButtonPressedEvent& event)
	{
		m_Camera->UpdateKeyState(event.GetMouseButton(), true);
		return true;
	}
}