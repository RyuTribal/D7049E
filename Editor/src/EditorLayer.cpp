#include "EditorLayer.h"
#include "Panels/Viewport.h"
#include "Panels/SceneGraph.h"
#include "Panels/ContentBrowser.h"
#include <imgui/imgui_internal.h>

namespace Editor {
	void EditorLayer::OnAttach()
	{
		HVE_ASSERT(m_Project->GetSettings().StartingScene != 0, "Starting scene is invalid!");
		OpenScene(m_Project->GetSettings().StartingScene);
	}

	void EditorLayer::OnUpdate(float delta_time)
	{
		Engine::Camera* curr_camera = m_Scene->GetCurrentCamera();
		Engine::Renderer::Get()->BeginFrame(curr_camera);
		if (EditorPanels::Viewport::IsFocused()) {
			m_Camera->Update(delta_time);
		}
		for (auto& line : m_DebugLines)
		{
			Renderer::Get()->SubmitLine(line);
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
				if (ImGui::MenuItem("New Project...", "Ctrl+P"))
				{

				}
				if (ImGui::MenuItem("Open Project...", "Ctrl+O"))
				{

				}

				ImGui::Separator();

				if (ImGui::MenuItem("New Scene", "Ctrl+N"))
				{

				}

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
				{
					SaveScene();
				}

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
				// Dont know yet, this is just more here for now
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
		}


		EditorPanels::SceneGraph::Render(m_Scene);

		ImGui::Begin("Content Browser");
			EditorPanels::ContentBrowser::Render(m_Scene);
		ImGui::End();

		ImGui::Begin("Viewport");
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

		ImGui::Begin("Stats");

		ImGui::Text("Renderer Stats:");
		ImGui::Text("FPS: %f", Renderer::Get()->GetStats()->frames_per_second);
		ImGui::Text("Draw Calls: %d", Renderer::Get()->GetStats()->draw_calls);
		ImGui::Text("Vertices: %d", Renderer::Get()->GetStats()->vertices_count);
		ImGui::Text("Indices: %d", Renderer::Get()->GetStats()->index_count);

		ImGui::End();
		ImGui::Begin("Settings");
		static bool ShouldDrawBoundingBoxes = false;
		ImGui::Text("Draw Mesh Bounding Boxes");
		ImGui::SameLine();
		if (ImGui::Checkbox("##boundboxesdraw", &ShouldDrawBoundingBoxes))
		{
			Renderer::Get()->SetDrawBoundingBoxes(ShouldDrawBoundingBoxes);
		}
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
		else if (control)
		{
			if (event.GetKeyCode() == KEY_S)
			{
				SaveScene();
			}
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
		// Too janky gotta fix a ray caster instead
		if (event.GetMouseButton() == MOUSE_BUTTON_LEFT && shift && EditorPanels::Viewport::IsHovered()) {
			auto [mouseX, mouseY] = EditorPanels::Viewport::GetMousePos();
			// HVE_INFO("Shot fired from {0} {1}", mouseX, mouseY);
			auto [origin, direction] = CastRay(mouseX, mouseY);

			Line line{};
			line.start = origin;
			line.end = direction;
			line.color = glm::vec4(1.f, 0.f, 0.f, 1.f);
			m_DebugLines.push_back(line); // Can be used for debugging

			auto meshEntities = m_Scene->GetAllEntitiesByType<MeshComponent>();

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
					bounding_box.TransformBy(submesh.WorldTransform);
					if (ray.IntersectsAABB(bounding_box, t))
					{
						const auto& triangleCache = component.mesh->GetMeshSource()->GetTriangleCache(submesh.Index);
						for (const auto& triangle : triangleCache)
						{

							glm::vec3 transformedV0 = glm::vec3(submesh.WorldTransform * glm::vec4(triangle.V0.coordinates, 1.0));
							glm::vec3 transformedV1 = glm::vec3(submesh.WorldTransform * glm::vec4(triangle.V1.coordinates, 1.0));
							glm::vec3 transformedV2 = glm::vec3(submesh.WorldTransform * glm::vec4(triangle.V2.coordinates, 1.0));

							if (ray.IntersectsTriangle(transformedV0, transformedV1, transformedV2, t))
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
		auto handle = m_Scene->CreateEntity("New Mesh Entity", nullptr);
		AssetHandle asset_handle = Project::GetActiveDesignAssetManager()->GetHandleByPath(file_path);
		Ref<MeshSource> mesh_source = AssetManager::GetAsset<MeshSource>(asset_handle);
		Ref<Mesh> mesh = CreateRef<Mesh>(mesh_source);
		MeshComponent mesh_comp{};
		mesh_comp.mesh = mesh;
		m_Scene->GetEntity(handle)->AddComponent<MeshComponent>(mesh_comp);
	}
	void EditorLayer::SaveScene()
	{
		std::filesystem::path scene_file_path = m_Project->GetSettings().AssetPath / std::filesystem::path("Scenes");
		if (AssetManager::GetMetadata(m_Scene->Handle))
		{
			scene_file_path = AssetManager::GetMetadata(m_Scene->Handle).FilePath;
		}
		m_Scene->SaveScene(Project::GetFullFilePath(scene_file_path));
	}
	void EditorLayer::OpenScene(AssetHandle handle)
	{
		auto scene = AssetManager::GetAsset<Scene>(handle);
		m_Camera = Engine::CreateRef<EditorCamera>(scene);
		scene->SetCurrentCamera(m_Camera->GetCamera());
		m_Scene = scene;
	}
}
