#pragma once
#include <Engine.h>
#include <imgui/imgui.h>
#include "EditorCamera.h"
#include <map>


using namespace Engine;


namespace Editor {
	class EditorLayer : public Layer {
	public:
		EditorLayer(std::string projectPath) : Layer("Editor"), m_ProjectPath(projectPath) {
			m_Settings = ProjectSerializer::Deserializer(m_ProjectPath);
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

	private:
		void SaveScene();

	private:
		Ref<Scene> m_Scene;
		std::string m_ProjectPath;
		Ref<EditorCamera> m_Camera;
		std::vector<EntityHandle*> entities{};
		bool b_EditDockspace = false;
		Ref<Framebuffer> m_SceneBuffer;
		ProjectSettings m_Settings{};
	};
}
