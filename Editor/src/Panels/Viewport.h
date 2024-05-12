#pragma once

#include <Engine.h>
#include <imgui/imgui.h>
#include <ImGui/ImGuizmo.h>

#include "SceneGraph.h"
#include <glm/gtc/type_ptr.hpp>

using namespace Engine;

namespace EditorPanels {
	class Viewport {
	public:
		static void Create() {
			if (!s_Instance) {
				s_Instance = new Viewport();
			}
		}

		static void Recreate()
		{
			s_Instance = new Viewport();
		}

		static void Render(Camera* camera) {
			Create();

			s_Instance->RenderImpl(camera);
		}
		static bool IsFocused() {
			Create();
			return s_Instance->m_Focused;
		}

		static bool IsHovered()
		{
			Create();
			return s_Instance->m_IsHovered;
		}

		static glm::vec2& GetViewportSize() { 
			Create();

			return s_Instance->m_ViewportSize; 
		}

		static std::pair<glm::vec2&, glm::vec2&> GetViewportBounds()
		{
			Create();
			return { s_Instance->m_ViewportBounds[0], s_Instance->m_ViewportBounds[1] };
		}

		static void OnKeyPressed(int keycode) {
			Create();
			s_Instance->OnKeyPressedImpl(keycode);
		}

		static bool CanReadPixelData() {
			Create();
			return (s_Instance->m_MouseX >= 0 && s_Instance->m_MouseY >= 0 && s_Instance->m_MouseX < (int)s_Instance->m_ViewportSize.x && s_Instance->m_MouseY < (int)s_Instance->m_ViewportSize.y);
		}

		static void SetUsingEditor(bool is_using)
		{
			Create();
			s_Instance->m_UsingEditor = is_using;
		}

		static std::pair<float, float> GetMousePos() {
			Create();
			auto [mx, my] = ImGui::GetMousePos();
			const auto& viewportBounds =  s_Instance->m_ViewportBounds;
			mx -= viewportBounds[0].x;
			my -= viewportBounds[0].y;
			auto viewportWidth = viewportBounds[1].x - viewportBounds[0].x;
			auto viewportHeight = viewportBounds[1].y - viewportBounds[0].y;

			return { (mx / viewportWidth) * 2.0f - 1.0f, ((my / viewportHeight) * 2.0f - 1.0f) * -1.0f };
		}

		void RenderImpl(Camera* camera) 
		{

			m_Focused = ImGui::IsWindowFocused();
			m_IsHovered = ImGui::IsWindowHovered();
			uint32_t id = Engine::Renderer::Get()->GetSceneTextureID();
			auto viewportOffset = ImGui::GetCursorPos();
			auto viewportSize = ImGui::GetContentRegionAvail();
			Engine::Renderer::Get()->ResizeViewport((uint32_t)viewportSize.x, (uint32_t)viewportSize.y);

			// Render viewport image
			ImGui::Image((void*)(intptr_t)(id), ImVec2{ viewportSize.x, viewportSize.y }, ImVec2(0, 1), ImVec2(1, 0));

			auto windowSize = ImGui::GetWindowSize();
			ImVec2 minBound = ImGui::GetWindowPos();

			ImVec2 maxBound = { minBound.x + windowSize.x, minBound.y + windowSize.y };
			m_ViewportBounds[0] = { minBound.x, minBound.y };
			m_ViewportBounds[1] = { maxBound.x, maxBound.y };

			Entity* selectedEntity = SceneGraph::GetSelectedEntity();
			if (selectedEntity != nullptr && m_UsingEditor)
			{
				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

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
		}

		void OnKeyPressedImpl(int keycode) {
			if (m_Focused && m_IsHovered)
			{
				switch (keycode)
				{
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
		}

	private:
		static Viewport* s_Instance;
		glm::vec2 m_ViewportSize;
		bool m_Focused;
		bool m_IsHovered;
		int m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
		glm::vec2 m_ViewportBounds[2];
		int m_MouseX = 0, m_MouseY = 0;
		bool m_UsingEditor = true;
	};
}
