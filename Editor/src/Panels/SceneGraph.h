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

		static void SetSelectedEntity(UUID id) {
			Create();
			s_Instance->SetSelectedEntityImpl(id);
		}

		static Ref<Scene> GetScene()
		{
			Create();
			return s_Instance->m_Scene;
		}

		static void SetScene(Ref<Scene> scene)
		{
			Create();
			s_Instance->m_SelectionContext = 0;
			s_Instance->m_Scene = scene;
		}

		static void DrawDropBox(const std::string& label);

	private:
		void SetActiveScene(Ref<Scene> scene) { m_Scene = scene; }
		void RenderImpl();
		void DisplaySceneEntity(SceneNode* node);
		Entity* GetSelectedEntityImpl() { return m_Scene->GetEntity(m_SelectionContext); }
		void SetSelectedEntityImpl(UUID id){ m_SelectionContext = id; }
		void DrawComponents();

		template<typename T>
		static void ShowMapSearchPopup(std::unordered_map<std::string, T>& map, bool use_first, std::string* result_destination);

		template<typename T>
		void DisplayAddComponentEntry(const std::string& entryName);

	private:
		Ref<Scene> m_Scene;
		static SceneGraph* s_Instance;
		UUID m_SelectionContext = 0;
	};
}
