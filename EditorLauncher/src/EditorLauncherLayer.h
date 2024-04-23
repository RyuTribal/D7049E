#pragma once
#include <Engine.h>
#include <imgui/imgui.h>
#include <map>


namespace EditorLauncher {
	class EditorLauncherLayer : public Engine::Layer {
	public:
		EditorLauncherLayer() : Layer("EditorLayer") {

		}
		~EditorLauncherLayer() = default;

		void OnAttach() override;
		
		void OnImGuiRender() override;

		void OnEvent(Engine::Event& event) override;

	private:
		void OpenProject(std::filesystem::path& project_path);

	private:
		char name_buffer[256];
		char path_buffer[1024];
	};
}
