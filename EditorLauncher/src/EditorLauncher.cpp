#include <Engine.h>
#include <Core/EntryPoint.h>
#include "EditorLauncherLayer.h"

class EditorLauncherApp : public Engine::Application
{
public:
	EditorLauncherApp(Engine::WindowProps props, Engine::ApplicationProps app_props) : Application(props, app_props)
	{
		PushLayer(new EditorLauncher::EditorLauncherLayer());
	}

	~EditorLauncherApp()
	{

	}
};

Engine::Application* Engine::CreateApplication(int argc, char** argv)
{
	WindowProps props{};
	props.Title = "Editor Launcher";
	props.Width = 1280;
	props.Height = 720;
	props.Fullscreen = false;
	props.FullScreenType = BORDERLESS;
	props.ScreenMaximized = false;
	props.VSync = false;
	props.Resizable = false;

	ApplicationProps app_props{};
	app_props.NoScripting = true;

	return new EditorLauncherApp(props, app_props);
}
