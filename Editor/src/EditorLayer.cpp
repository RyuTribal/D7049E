#include "EditorLayer.h"
#include "Panels/Viewport.h"
#include "Panels/SceneGraph.h"
#include "Panels/ContentBrowser.h"
#include "Panels/ProjectSettings.h"
#include "Panels/SceneSettings.h"
#include <imgui/imgui_internal.h>

namespace Editor {
	void EditorLayer::OnAttach()
	{
		m_Camera = Engine::CreateRef<EditorCamera>(m_CurrentScene);
	}

	void EditorLayer::OnUpdate(float delta_time)
	{

		Engine::Camera* curr_camera = nullptr;
		if (m_SceneState == SceneState::Play)
		{
			curr_camera = m_CurrentScene->GetPrimaryEntityCamera();
		}
		else
		{
			curr_camera = m_Camera->GetCamera().get();
		}
		Engine::Renderer::Get()->BeginFrame(curr_camera);
		if (EditorPanels::Viewport::IsFocused()) {
			m_Camera->Update(delta_time);
		}

		if (m_SceneState == SceneState::Edit)
		{
			auto entity = EditorPanels::SceneGraph::GetSelectedEntity();
			if (entity)
			{
				if (entity->HasComponent<BoxColliderComponent>())
				{
					auto collider = entity->GetComponent<BoxColliderComponent>();
					DebugBox debug_box{};
					auto transform = entity->GetComponent<TransformComponent>()->world_transform;
					transform.translation += collider->Offset;
					debug_box.Transform = transform.mat4();
					debug_box.Color = glm::vec4(1.f, 0.f, 0.f, 1.f);
					debug_box.Size = collider->HalfSize;
					Renderer::Get()->SubmitDebugBox(debug_box);
				}


				if (entity->HasComponent<SphereColliderComponent>())
				{
					auto collider = entity->GetComponent<SphereColliderComponent>();
					DebugSphere debug_sphere{};
					auto transform = entity->GetComponent<TransformComponent>()->world_transform;
					transform.translation += collider->Offset;
					debug_sphere.Transform = transform.mat4();
					debug_sphere.Color = glm::vec4(1.f, 0.f, 0.f, 1.f);
					debug_sphere.Radius = collider->Radius;
					Renderer::Get()->SubmitDebugSphere(debug_sphere);
				}

				if (entity->HasComponent<CharacterControllerComponent>())
				{
					auto collider = entity->GetComponent<CharacterControllerComponent>();
					DebugCapsule debug_capsule{};
					auto transform = entity->GetComponent<TransformComponent>()->world_transform;
					transform.translation += collider->Offset;
					debug_capsule.Transform = transform.mat4();
					debug_capsule.Color = glm::vec4(1.f, 0.f, 0.f, 1.f);
					debug_capsule.Radius = collider->Radius;
					debug_capsule.HalfHeight = collider->HalfHeight;
					Renderer::Get()->SubmitDebugCapsule(debug_capsule);
				}
			}
		}
		
		m_CurrentScene->UpdateScene();
		Engine::Renderer::Get()->EndFrame();

		if (ScriptEngine::ShouldReload() && m_SceneState != SceneState::Play)
		{
			Project::ReloadScripts();
		}
	}

	void EditorLayer::OnDetach()
	{
		EditorPanels::ProjectSettings::Shutdown();
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

		if (m_SceneState == SceneState::Play)
		{
			window_flags |= ImGuiWindowFlags_NoInputs;
		}

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
				if (ImGui::MenuItem("New Project...", "Ctrl+P"))
				{
					ImGui::OpenPopup("Create Project");
					m_CurrentMenuAction = MenuAction::NewProject;
				}
				if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
				{
					BeginOpenProject();
				}

				if (ImGui::MenuItem("Save Project", "Ctrl+S"))
				{
					SaveProject();
				}

				// ImGui::Separator();

				//if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				//{
				//	//ImGui::OpenPopup("Create Scene");
				//}

				//if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
				//{
				// Dont know if we will need this
				//}

				ImGui::Separator();

				if (ImGui::MenuItem("Exit"))
					Engine::Application::Get().Close();

				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Edit"))
			{
				if (ImGui::MenuItem("Recreate Script Project"))
				{
					Project::CreateScriptProject();
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Project"))
			{
				if (ImGui::MenuItem("Reload Scripts"))
				{
					Project::ReloadScripts();
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}

		if (m_CurrentMenuAction == MenuAction::NewProject) ImGui::OpenPopup("Create Project");


		if (ImGui::BeginPopupModal("Create Project"))
		{
			ImGui::SetNextWindowSize({ 600, 338 });
			ImGui::Columns(2, "CreateColumns", false);
			ImGui::SetColumnWidth(0, 200);
			ImGui::Text("Project Name");
			ImGui::Text("Project Directory");
			ImGui::NextColumn();

			ImGui::InputText("##Name", m_NewProjectNameBuffer, sizeof(m_NewProjectNameBuffer));
			ImGui::InputText("##Dir", m_NewProjectPathBuffer, sizeof(m_NewProjectPathBuffer));
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				auto [success, path] = Engine::FilePicker::OpenFileExplorer(true);
				if (success)
				{
					strncpy(m_NewProjectPathBuffer, path.c_str(), sizeof(m_NewProjectPathBuffer));
					m_NewProjectPathBuffer[sizeof(m_NewProjectPathBuffer) - 1] = '\0';
				}
			}

			ImGui::Columns(1);

			static bool successfully_created = false;
			static bool attempted_to_create = false;

			if (ImGui::Button("Create"))
			{
				auto [success, project_file_path] = Engine::ProjectSerializer::CreateNewProject(m_NewProjectPathBuffer, m_NewProjectNameBuffer);
				successfully_created = success;
				attempted_to_create = true;
				if (success)
				{
					OpenNewProject(project_file_path.string());
					ImGui::CloseCurrentPopup();
					successfully_created = false;
					attempted_to_create = false;
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			if (!successfully_created && attempted_to_create)
			{
				ImGui::Text("Error: Project creation failed. Please check the logs!");
			}

			ImGui::EndPopup();
		}


		EditorPanels::SceneGraph::Render(m_CurrentScene);

		ImGui::Begin("Content Browser");
			EditorPanels::ContentBrowser::Render(m_CurrentScene);
		ImGui::End();

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
		ImGui::Begin("Viewport", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

		EditorPanels::Viewport::Render(m_Camera->GetCamera().get());

		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				auto droppedPath = *(std::filesystem::path*)payload->Data;
				HVE_CORE_INFO("Dropped file: {0}", droppedPath.string());
				if (Project::GetActiveDesignAssetManager()->GetAssetTypeFromFileExtension(droppedPath.extension()) == AssetType::MeshSource)
				{
					CreateEntityFromMesh(droppedPath);
				}
			}
			ImGui::EndDragDropTarget();
		}
		ImGui::End();

		ImGui::PopStyleVar(1);

		ImGui::Begin("Stats");

		ImGui::Text("Renderer Stats:");
		ImGui::Text("FPS: %f", Renderer::Get()->GetStats()->frames_per_second);
		ImGui::Text("Draw Calls: %d", Renderer::Get()->GetStats()->draw_calls);
		ImGui::Text("Vertices: %d", Renderer::Get()->GetStats()->vertices_count);
		ImGui::Text("Indices: %d", Renderer::Get()->GetStats()->index_count);

		ImGui::End();

		ImGui::Begin("Project Settings");
		EditorPanels::ProjectSettings::Render(m_Camera->GetCamera());
		ImGui::End();

		ImGui::Begin("Scene Settings");
		EditorPanels::SceneSettings::Render(m_CurrentScene);
		ImGui::End();

		UIToolBar();

		ImGui::End();

		m_CurrentMenuAction = MenuAction::None;

	}

	void EditorLayer::UIToolBar()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 2));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, ImVec2(0, 0));
		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
		auto& colors = ImGui::GetStyle().Colors;
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(colors[ImGuiCol_ButtonHovered].x, colors[ImGuiCol_ButtonHovered].y, colors[ImGuiCol_ButtonHovered].z, 0.5f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(colors[ImGuiCol_ButtonHovered].x, colors[ImGuiCol_ButtonHovered].y, colors[ImGuiCol_ButtonHovered].z, 0.5f));

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
		float size = ImGui::GetWindowHeight() - 4.0f;
		/*size *= 2;
		size -= 4.0f;*/
		Ref<Texture2D> icon;

		switch (m_SceneState)
		{
			case SceneState::Edit:
				icon = EditorResources::FileIcons["play"];
				EditorPanels::Viewport::SetUsingEditor(true);
				break;
			case SceneState::Play:
				icon = EditorResources::FileIcons["stop"];
				EditorPanels::Viewport::SetUsingEditor(false);
				break;
			default:
				icon = EditorResources::FileIcons["play"];
				EditorPanels::Viewport::SetUsingEditor(true);
		}

		ImGui::SetCursorPosX((ImGui::GetWindowContentRegionMax().x * 0.5f) - (size * 0.5f));
		if (ImGui::ImageButton((ImTextureID)icon->GetRendererID(), ImVec2(size, size), ImVec2(0, 0), ImVec2(1, 1), 0))
		{
			if (m_SceneState == SceneState::Edit)
			{
				OnScenePlay();
			}
			else if (m_SceneState == SceneState::Play)
			{
				OnSceneStop();
			}
		}

		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);

		ImGui::End();
	}

	void EditorLayer::OnScenePlay()
	{
		m_SceneState = SceneState::Play;

		m_CurrentScene = Scene::Copy(m_EditorScene);

		m_CurrentScene->OnRuntimeStart();

		Input::SetLockMouseMode(true);
	}

	void EditorLayer::OnSceneStop()
	{
		m_SceneState = SceneState::Edit;
		m_CurrentScene->OnRuntimeStop();
		m_CurrentScene = m_EditorScene;
		Input::SetLockMouseMode(false);
	}

	bool EditorLayer::OnKeyPress(Engine::KeyPressedEvent& event)
	{
		bool control = Input::IsKeyPressed(KEY_LEFT_CONTROL) || Input::IsKeyPressed(KEY_RIGHT_CONTROL);
		bool shift = Input::IsKeyPressed(KEY_LEFT_SHIFT) || Input::IsKeyPressed(KEY_RIGHT_SHIFT);

		if (m_SceneState == SceneState::Play)
		{
			if (shift && event.GetKeyCode() == KEY_ESCAPE)
			{
				OnSceneStop();
				return false;
			}
			return false;
		}

		if (shift) {
			m_Camera->UpdateKeyState(event.GetKeyCode(), true);
		}
		else if (control)
		{
			switch (event.GetKeyCode())
			{
				case KEY_S: SaveProject();									break;
				case KEY_O: BeginOpenProject();								break;
				case KEY_P: m_CurrentMenuAction = MenuAction::NewProject;	break;
			}
		}
		else {
			EditorPanels::Viewport::OnKeyPressed(event.GetKeyCode());
		}
		return true;
	}

	bool EditorLayer::OnKeyRelease(Engine::KeyReleasedEvent& event)
	{
		if (m_SceneState == SceneState::Play)
		{
			return false;
		}
		m_Camera->UpdateKeyState(event.GetKeyCode(), false);
		return true;
	}

	bool EditorLayer::OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& event)
	{
		if (m_SceneState == SceneState::Play)
		{
			return false;
		}
		m_Camera->UpdateKeyState(event.GetMouseButton(), false);
		return true;
	}

	bool EditorLayer::OnMouseMoved(Engine::MouseMovedEvent& event)
	{
		if (m_SceneState == SceneState::Play)
		{
			return false;
		}
		m_Camera->PanCamera();
		return true;
	}

	bool EditorLayer::OnMouseButtonPressed(Engine::MouseButtonPressedEvent& event)
	{
		if (m_SceneState == SceneState::Play)
		{

			return false;
		}

		bool shift = Input::IsKeyPressed(KEY_LEFT_SHIFT) || Input::IsKeyPressed(KEY_RIGHT_SHIFT);
		if (EditorPanels::Viewport::IsHovered())
		{
			m_Camera->UpdateKeyState(event.GetMouseButton(), true);
		}
		// Too janky gotta fix a ray caster instead
		if (event.GetMouseButton() == MOUSE_BUTTON_LEFT && shift && EditorPanels::Viewport::IsHovered()) {
			auto [mouseX, mouseY] = EditorPanels::Viewport::GetMousePos();
			// HVE_INFO("Shot fired from {0} {1}", mouseX, mouseY);
			auto [origin, direction] = CastRay(mouseX, mouseY);

			auto meshEntities = m_CurrentScene->GetAllEntitiesByType<MeshComponent>();

			std::vector<SelectionData> selected_entities{};

			for (const auto& [entity, component] : *meshEntities)
			{
				if (!component.mesh)
				{
					continue;
				}

				for (const auto& submesh : component.mesh->GetMeshSource()->GetSubmeshes())
				{
					
					Math::Ray ray = {
						glm::inverse(submesh.WorldTransform) * glm::vec4(origin, 1.0f),
						glm::inverse(glm::mat3(submesh.WorldTransform)) * direction
					};

					float t;
					Math::BoundingBox bounding_box = submesh.Bounds;
					if (ray.IntersectsAABB(bounding_box, t))
					{
						const auto& triangleCache = component.mesh->GetMeshSource()->GetTriangleCache(submesh.Index);
						for (const auto& triangle : triangleCache)
						{

							if (ray.IntersectsTriangle(triangle.V0.coordinates, triangle.V1.coordinates, triangle.V2.coordinates, t))
							{
								selected_entities.push_back({ entity, t });
								break;
							}
						}
					}
				}

			}

			std::sort(selected_entities.begin(), selected_entities.end(), [](auto& a, auto& b) { return a.Distance < b.Distance; });

			if (selected_entities.size() > 0)
			{
				auto selected_entity = EditorPanels::SceneGraph::GetSelectedEntity();
				if (selected_entity && selected_entity->GetHandle()->GetID() == selected_entities[0].entity.GetID())
				{
					EditorPanels::SceneGraph::SetSelectedEntity(0);
				}
				else
				{
					EditorPanels::SceneGraph::SetSelectedEntity(selected_entities[0].entity.GetID());
				}
			}
			else
			{
				EditorPanels::SceneGraph::SetSelectedEntity(0);
			}
			

		}
		return true;
	}
	bool EditorLayer::OnScrolled(MouseScrolledEvent& event)
	{
		if (m_SceneState == SceneState::Play)
		{
			return false;
		}
		if (EditorPanels::Viewport::IsHovered())
		{
			m_Camera->Zoom(event.GetYOffset());
		}
		return false;
	}


	std::pair<glm::vec3, glm::vec3> EditorLayer::CastRay(float mx, float my)
	{
		glm::vec4 mouseClipPos = { mx, my, -1.0f, 1.0f };
		auto raw_camera = m_Camera->GetCamera();

		auto inverseProj = glm::inverse(raw_camera->GetProjection());
		auto inverseView = glm::inverse(glm::mat3(raw_camera->GetView()));

		glm::vec4 ray = inverseProj * mouseClipPos;
		glm::vec3 rayPos = raw_camera->CalculatePosition();
		glm::vec3 rayDir = inverseView * glm::vec3(ray);

		return { rayPos, rayDir };
	}

	void EditorLayer::CreateEntityFromMesh(const std::filesystem::path& file_path)
	{
		auto handle = m_CurrentScene->CreateEntity("New Mesh Entity", nullptr);
		AssetHandle asset_handle = Project::GetActiveDesignAssetManager()->GetHandleByPath(file_path);
		Ref<MeshSource> mesh_source = AssetManager::GetAsset<MeshSource>(asset_handle);
		Ref<Mesh> mesh = CreateRef<Mesh>(mesh_source);
		MeshComponent mesh_comp{};
		mesh_comp.mesh = mesh;
		m_CurrentScene->GetEntity(handle)->AddComponent<MeshComponent>(mesh_comp);
	}

	void EditorLayer::OpenNewProject(const std::string& project_path)
	{
		m_Project = Project::Load(project_path);
		HVE_ASSERT(m_Project->GetSettings().StartingScene != 0, "Starting scene is invalid!");
		OpenScene(m_Project->GetSettings().StartingScene);

		memset(m_NewProjectNameBuffer, 0, sizeof(m_NewProjectNameBuffer));
		memset(m_NewProjectPathBuffer, 0, sizeof(m_NewProjectPathBuffer));

		EditorPanels::SceneGraph::SetScene(m_CurrentScene);
		EditorPanels::ProjectSettings::Init();

		Project::ReloadScripts();

		HVE_WARN(Project::GetActive()->GetSettings().RootPath.string());

		std::string new_title = std::filesystem::path(project_path).stem().string() + " - Editor";
		Application::Get().GetWindow().SetTitle(new_title);

		EditorPanels::ContentBrowser::Recreate();
	}

	void EditorLayer::BeginOpenProject()
	{
		std::string file_ending = "hveproject";
		std::vector<std::vector<std::string>> filter = { {"Helios project files", file_ending} };
		auto [success, path] = Engine::FilePicker::OpenFileExplorer(filter, false);
		if (success)
		{
			strncpy(m_NewProjectPathBuffer, path.c_str(), sizeof(m_NewProjectPathBuffer));
			m_NewProjectPathBuffer[sizeof(m_NewProjectPathBuffer) - 1] = '\0';
			OpenNewProject(std::string(m_NewProjectPathBuffer));
		}
		Input::ClearKeyStates(); // Needed because if you open it through the shortcuts, since it locks the thread, the keystate for control will stay as pressed and not as released
	}

	void EditorLayer::SaveProject()
	{
		Project::SaveActive();
		std::filesystem::path scene_file_path = m_Project->GetSettings().AssetPath / std::filesystem::path("Scenes");
		if (AssetManager::GetMetadata(m_EditorScene->Handle))
		{
			scene_file_path = AssetManager::GetMetadata(m_EditorScene->Handle).FilePath;
		}
		m_EditorScene->SaveScene(Project::GetFullFilePath(scene_file_path));
	}

	void EditorLayer::OpenScene(AssetHandle handle)
	{
		auto scene = AssetManager::GetAsset<Scene>(handle);
		m_CurrentScene = scene;
		m_EditorScene = scene;
		if (m_Camera)
		{
			m_Camera->UpdateContext(scene);
		}
	}
}
