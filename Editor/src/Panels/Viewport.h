#pragma once

#include <Engine.h>
#include <imgui/imgui.h>
#include <ImGui/ImGuizmo.h>

#include "SceneGraph.h"
#include <glm/gtc/type_ptr.hpp>

namespace EditorPanels {
	class Viewport {
	public:
		static void Create() {
			if (!s_Instance) {
				s_Instance = new Viewport();
			}
		}
		static void Render(Camera* camera) {
			Create();

			s_Instance->RenderImpl(camera);
		}
		static bool IsFocused() {
			Create();
			return s_Instance->m_Focused;
		}

		static glm::vec2& GetViewportSize() { 
			Create();

			return s_Instance->m_ViewportSize; 
		}

		static void OnKeyPressed(int keycode) {
			Create();
			s_Instance->OnKeyPressedImpl(keycode);
		}

		static void ActivateGizmo() {
			Create();
			if (s_Instance->m_GizmoType == -1) {
				s_Instance->m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			}
		}

		static bool CanReadPixelData() {
			Create();
			return (s_Instance->m_MouseX >= 0 && s_Instance->m_MouseY >= 0 && s_Instance->m_MouseX < (int)s_Instance->m_ViewportSize.x && s_Instance->m_MouseY < (int)s_Instance->m_ViewportSize.y);
		}

		static std::pair<int, int> GetMousePos() {
			Create();
			return { s_Instance->m_MouseX, s_Instance->m_MouseY };
		}

		void RenderImpl(Camera* camera) {
			ImGui::Begin("Viewport");

			auto [mx, my] = ImGui::GetMousePos();
			mx -= m_ViewportBounds[0].x;
			my -= m_ViewportBounds[0].y;
			my = m_ViewportSize.y - my;
			m_MouseX = (int)mx;
			m_MouseY = (int)my;

			m_Focused = ImGui::IsWindowFocused();
			uint32_t id = Engine::Renderer::Get()->GetSceneTextureID();
			ImVec2 windowSize = ImGui::GetContentRegionAvail();
			if (m_ViewportSize != *((glm::vec2*)&windowSize)) {
				Engine::Renderer::Get()->ResizeViewport((uint32_t)windowSize.x, (uint32_t)windowSize.y);
				m_ViewportSize = { windowSize.x, windowSize.y };
			}
			ImGui::Image((void*)(intptr_t)(id), ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));

			Entity* selectedEntity = SceneGraph::GetSelectedEntity();
			if (selectedEntity != nullptr && m_GizmoType != -1)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
				auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
				auto viewportOffset = ImGui::GetWindowPos();
				m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
				m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };
				ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y, m_ViewportBounds[1].x - m_ViewportBounds[0].x, m_ViewportBounds[1].y - m_ViewportBounds[0].y);

				const glm::mat4& cameraProjection = camera->GetProjection();
				glm::mat4 cameraView = camera->GetView();

				auto tc = selectedEntity->GetComponent<TransformComponent>();
				glm::mat4 transform = tc->local_transform.mat4();

				bool snap = Input::IsKeyPressed(KEY_LEFT_CONTROL);
				float snapValue = 0.5f;
				if (m_GizmoType == ImGuizmo::OPERATION::ROTATE)
					snapValue = 45.0f;

				float snapValues[3] = { snapValue, snapValue, snapValue };

				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform),
					nullptr, snap ? snapValues : nullptr);

				if (ImGuizmo::IsUsing())
				{
					glm::vec3 translation, rotation, scale;
					Math::DecomposeTransform(transform, translation, rotation, scale);

					glm::vec3 deltaRotation = rotation - tc->local_transform.rotation;
					tc->local_transform.translation = translation;
					tc->local_transform.rotation += deltaRotation;
					tc->local_transform.scale = scale;
				}
			}

			ImGui::End();
		}

		void OnKeyPressedImpl(int keycode) {
			switch (keycode) {
			case KEY_Q:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = -1;
				break;
			}
			case KEY_W:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			}
			case KEY_E:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			}
			case KEY_R:
			{
				if (!ImGuizmo::IsUsing())
					m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			}
			}
		}

	private:
		static Viewport* s_Instance;
		glm::vec2 m_ViewportSize;
		bool m_Focused;
		int m_GizmoType = -1;
		glm::vec2 m_ViewportBounds[2];
		int m_MouseX = 0, m_MouseY = 0;

	};
}