#pragma once
#include <Engine.h>
#include <imgui/imgui.h>
#include "Primitives/Cuboid.h"
#include "Materials/Silver.h"
#include "EditorCamera.h"
#include <map>


namespace Editor {
	class EditorLayer : public Engine::Layer {
	public:
		EditorLayer() : Layer("Editor") {

		}
		~EditorLayer() = default;

		void OnAttach() override;
		void OnUpdate(float delta_time) override;
		void OnEvent(Engine::Event& event) override;
		void OnImGuiRender() override;
		bool OnKeyPress(Engine::KeyPressedEvent& event);
		bool OnKeyRelease(Engine::KeyReleasedEvent& event);
		bool OnMouseButtonReleased(Engine::MouseButtonReleasedEvent& event);
		bool OnMouseMoved(Engine::MouseMovedEvent& event);
		bool OnMouseButtonPressed(Engine::MouseButtonPressedEvent& event);

	private:
		Engine::Ref<Engine::Scene> m_Scene;
		Engine::Ref<EditorCamera> m_Camera;
		std::vector<Engine::EntityHandle*> entities{};
		bool b_EditDockspace = false;
	};
}