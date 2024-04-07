#pragma once
#include <Engine.h>
#include <imgui/imgui.h>
#include "Primitives/Cuboid.h"
#include "Materials/Silver.h"
#include "EditorCamera.h"
#include <map>


using namespace Engine;


namespace Editor {
	class EditorLayer : public Layer {
	public:
		EditorLayer() : Layer("Editor") {

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
		Ref<Scene> m_Scene;
		Ref<EditorCamera> m_Camera;
		std::vector<EntityHandle*> entities{};
		bool b_EditDockspace = false;
		Ref<Framebuffer> m_SceneBuffer;
	};
}