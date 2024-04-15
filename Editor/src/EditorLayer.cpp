#include "EditorLayer.h"
#include "Panels/Viewport.h"
#include "Panels/SceneGraph.h"
#include <imgui/imgui_internal.h>

namespace Editor {
	void EditorLayer::OnAttach()
	{
		auto [scene, camera_entity_handle] = Engine::Scene::CreateScene("Editor Scene");
		m_Camera = Engine::CreateRef<EditorCamera>(scene->GetCurrentCamera(), camera_entity_handle, scene);
		m_Scene = scene;
		Engine::Renderer::Get()->SetBackgroundColor(0, 0, 0);

		Engine::EntityHandle* object_entity_handle = m_Scene->CreateEntity("Lion", nullptr);
		Engine::Entity* cube_entity = m_Scene->GetEntity(object_entity_handle);

		Ref<Mesh> object_mesh = ModelLibrary::Get()->CreateMesh("C:/Users/sedel/Downloads/Lion/source/lion/mn_rpat_00_ani.FBX", &object_entity_handle->GetID(), std::string(ROOT_PATH) + "/shaders/default_static_shader");
		cube_entity->GetComponent<TransformComponent>()->local_transform.scale = glm::vec3(0.01f, 0.01f, 0.01f);

		cube_entity->AddComponent<Engine::MeshComponent>(object_mesh);

		entities.push_back(object_entity_handle);


		Engine::EntityHandle* light_entity_handle = m_Scene->CreateEntity("Sun", nullptr);
		Engine::Entity* light_entity = m_Scene->GetEntity(light_entity_handle);
		Engine::Ref<Engine::DirectionalLight> light = Engine::CreateRef<Engine::DirectionalLight>(); // defaults to white

		light_entity->GetComponent<TransformComponent>()->local_transform.translation = glm::vec3(0.5f, 0.7f, 0.f);
		Engine::DirectionalLightComponent new_light(light);

		light_entity->AddComponent<Engine::DirectionalLightComponent>(new_light);

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

		dispatcher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnKeyPress));
		dispatcher.Dispatch<KeyReleasedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnKeyRelease));
		dispatcher.Dispatch<MouseButtonPressedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnMouseButtonPressed));
		dispatcher.Dispatch<MouseButtonReleasedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnMouseButtonReleased));
		dispatcher.Dispatch<MouseMovedEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnMouseMoved));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(Editor::EditorLayer::OnScrolled));
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


		EditorPanels::SceneGraph::Render(m_Scene);

		ImGui::Begin("Content Browser");

		ImGui::End();

		EditorPanels::Viewport::Render(m_Camera->GetCamera());

		ImGui::Begin("Stats");

		ImGui::Text("Renderer Stats:");
		ImGui::Text("FPS: %f", Renderer::Get()->GetStats()->frames_per_second);
		ImGui::Text("Draw Calls: %d", Renderer::Get()->GetStats()->draw_calls);
		ImGui::Text("Vertices: %d", Renderer::Get()->GetStats()->vertices_count);
		ImGui::Text("Indices: %d", Renderer::Get()->GetStats()->index_count);

		ImGui::End();

		ImGui::Begin("Settings");

		ImGui::End();

		ImGui::End();

	}

	bool EditorLayer::OnKeyPress(Engine::KeyPressedEvent& event)
	{

		bool control = Input::IsKeyPressed(KEY_LEFT_CONTROL) || Input::IsKeyPressed(KEY_RIGHT_CONTROL);
		bool shift = Input::IsKeyPressed(KEY_LEFT_SHIFT) || Input::IsKeyPressed(KEY_RIGHT_SHIFT);

		if (shift) {
			m_Camera->UpdateKeyState(event.GetKeyCode(), true);
		}
		else {
			EditorPanels::Viewport::OnKeyPressed(event.GetKeyCode());
		}
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
		bool shift = Input::IsKeyPressed(KEY_LEFT_SHIFT) || Input::IsKeyPressed(KEY_RIGHT_SHIFT);
		if (EditorPanels::Viewport::IsHovered())
		{
			m_Camera->UpdateKeyState(event.GetMouseButton(), true);
		}
		
		if (event.GetMouseButton() == MOUSE_BUTTON_LEFT && shift && EditorPanels::Viewport::IsFocused()) {
			auto [x, y] = EditorPanels::Viewport::GetMousePos();
			int pixelData = Renderer::Get()->GetObjectFrameBuffer()->ReadPixel(1, x, y);
			if (pixelData != -1 || EditorPanels::SceneGraph::GetSelectedEntity()->GetID() != (UUID)pixelData) {
				EditorPanels::SceneGraph::SetSelectedEntity(pixelData);
				EditorPanels::Viewport::ActivateGizmo();
			}
		}
		return true;
	}
	bool EditorLayer::OnScrolled(MouseScrolledEvent& event)
	{
		if (EditorPanels::Viewport::IsHovered())
		{
			m_Camera->Zoom(event.GetYOffset());
		}
		return false;
	}
}
