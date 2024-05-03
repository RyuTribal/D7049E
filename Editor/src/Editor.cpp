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
	std::string projectPath;
	if (argc > 0 && std::filesystem::path(argv[0]).filename().extension() == ".hveproject")
	{
		projectPath = argv[0];
	}
	else if (argc > 1 && std::filesystem::path(argv[1]).filename().extension() == ".hveproject")
	{
		projectPath = argv[1];
	}
	else
	{
		std::filesystem::path full_sandbox_path = std::filesystem::current_path() / "Sandbox/Sandbox.hveproject";
		projectPath = full_sandbox_path.string();
	}
	 
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
