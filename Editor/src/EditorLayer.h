#pragma once
#include <Engine.h>
#include <imgui/imgui.h>
#include "EditorCamera.h"
#include <map>
#include "EditorResources.h"


using namespace Engine;


namespace Editor {

	enum class SceneState
	{
		Edit = 0, Play = 1
	};

	struct SelectionData
	{
		EntityHandle entity;
		float Distance = 0.0f;
	};

	class EditorLayer : public Layer {
	public:
		EditorLayer(std::string projectPath) : Layer("Editor"), m_ProjectPath(projectPath) {
			m_Project = Project::Load(m_ProjectPath);
			EditorResources::Init();
		}
		~EditorLayer() = default;

		void OnAttach() override;
		void OnUpdate(float delta_time) override;
		void OnEvent(Event& event) override;
		void OnImGuiRender() override;
		bool OnKeyPress(KeyPressedEvent& event);
		bool OnKeyRelease(KeyReleasedEvent& event);
		bool OnMouseButtonReleased(MouseButtonReleasedEvent& event);
		bool OnMouseMoved(MouseMovedEvent& event);
		bool OnMouseButtonPressed(MouseButtonPressedEvent& event);
		bool OnScrolled(MouseScrolledEvent& event);

		void CreateEntityFromMesh(const std::filesystem::path& file_path);



	private:
		void SaveScene();
		void OpenScene(AssetHandle handle);
		void UIToolBar();
		void OnScenePlay();
		void OnSceneStop();

		std::pair<glm::vec3, glm::vec3> CastRay(float mx, float my);

	private:
		Ref<Scene> m_CurrentScene;
		Ref<Scene> m_EditorScene;
		std::string m_ProjectPath;
		Ref<EditorCamera> m_Camera;
		std::vector<EntityHandle*> entities{};
		bool b_EditDockspace = true;
		Ref<Framebuffer> m_SceneBuffer;
		Ref<Project> m_Project;

		std::vector<Line> m_DebugLines;
		SceneState m_SceneState = SceneState::Edit;
	};
}
