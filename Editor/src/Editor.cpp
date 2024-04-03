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
	props.VSync = true; // Keep it for now so we can have faster movement I was too lazy to fix the mouse movement to properly work without vsync
	return new EditorApp(props);
}
