#pragma once
#include <Engine.h>
#include <imgui/imgui.h>

using namespace Engine;

namespace EditorPanels {
	class SceneGraph {
	public:
		static void Create() {
			if (!s_Instance) {
				s_Instance = new SceneGraph();
			}
		}
		static void Render(Ref<Scene> scene) {
			Create();
			s_Instance->SetActiveScene(scene);
			s_Instance->RenderImpl();
		}

		static Entity* GetSelectedEntity() {
			Create();
			return s_Instance->GetSelectedEntityImpl();
		}

		static void SetSelectedEntity(int id) {
			Create();
			s_Instance->SetSelectedEntityImpl(id);
		}

	private:
		void SetActiveScene(Ref<Scene> scene) { m_Scene = scene; }
		void RenderImpl();
		void DisplaySceneEntity(SceneNode* node);
		Entity* GetSelectedEntityImpl() { return m_Scene->GetEntity(m_SelectionContext); }
		void SetSelectedEntityImpl(int id){ m_SelectionContext = (UUID)id; }

	private:
		Ref<Scene> m_Scene;
		static SceneGraph* s_Instance;
		UUID m_SelectionContext;
	};
}