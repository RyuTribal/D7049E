#include <Engine.h>
#include <Core/EntryPoint.h>
#include "EditorLayer.h"

class EditorApp : public Engine::Application
{
public:
	EditorApp(Engine::WindowProps props) : Application(props)
	{
		PushLayer(new Editor::EditorLayer());
	}

	~EditorApp()
	{

	}
};

Engine::Application* Engine::CreateApplication()
{
	WindowProps props{};
	props.Title = "Editor";
	props.Width = 1280;
	props.Height = 720;
	props.Fullscreen = false;
	props.FullScreenType = BORDERLESS;
	props.ScreenMaximized = true;	
	props.VSync = false;
	return new EditorApp(props);
}
