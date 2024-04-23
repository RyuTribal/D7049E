#include <Engine.h>
#include <Core/EntryPoint.h>
#include "EditorLayer.h"

class EditorApp : public Engine::Application
{
public:
	EditorApp(Engine::WindowProps props, std::string projectPath) : Application(props)
	{
		PushLayer(new Editor::EditorLayer(projectPath));
	}

	~EditorApp()
	{

	}
};

Engine::Application* Engine::CreateApplication(int argc, char** argv)
{

	HVE_ASSERT(argc > 1, "No project selected. Please use the launcher to open a project!");
	std::string projectPath = argv[1];
	std::filesystem::path project_name = std::filesystem::path(projectPath).filename().stem();
	WindowProps props{};
	props.Title = project_name.string() + " - Editor";
	props.Width = 1280;
	props.Height = 720;
	props.Fullscreen = false;
	props.FullScreenType = BORDERLESS;
	props.ScreenMaximized = true;	
	props.VSync = false;
	return new EditorApp(props, projectPath);
}
