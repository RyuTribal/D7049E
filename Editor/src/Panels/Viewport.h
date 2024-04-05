#pragma once

#include <Engine.h>
#include <imgui/imgui.h>

namespace EditorPanels {
	class Viewport {
	public:
		static void Create() {
			if (!s_Instance) {
				s_Instance = new Viewport();
			}
		}
		static void Render() {
			Create();

			s_Instance->RenderImpl();
		}
		static bool IsFocused() {
			Create();
			return s_Instance->m_Focused;
		}
		void RenderImpl() {
			ImGui::Begin("Viewport");
			m_Focused = ImGui::IsWindowFocused();
			uint32_t id = Engine::Renderer::Get()->GetSceneTextureID();
			ImVec2 windowSize = ImGui::GetContentRegionAvail();
			if (m_ViewportSize != *((glm::vec2*)&windowSize)) {
				Engine::Renderer::Get()->ResizeViewport((uint32_t)windowSize.x, (uint32_t)windowSize.y);
				m_ViewportSize = { windowSize.x, windowSize.y };
			}
			ImGui::Image((void*)(intptr_t)(id), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));
			ImGui::End();
		}
	private:
		static Viewport* s_Instance;
		glm::vec2 m_ViewportSize;
		bool m_Focused;
	};
}