#include <Engine.h>
#include <Core/EntryPoint.h>
#include "EditorLayer.h"

class EditorApp : public Engine::Application
{
public:
	EditorApp(Engine::WindowProps props) : Application(props)
	{
	    auto [camera_entity, scene] = Engine::Scene::CreateScene("Scene1");
		entities[camera_entity->GetID()] = camera_entity;
	    Engine::Renderer::Get()->SetBackgroundColor(128, 128, 128); //You can change the color here to see that it updates
		PushLayer(new Editor::EditorLayer(scene));
	}

	~EditorApp()
	{

	}

	std::unordered_map <Engine::UUID, Engine::Ref<Engine::Entity>> entities{}; //To make sure the entities are not garbage collected
};

Engine::Application* Engine::CreateApplication()
{
	WindowProps props{};
	props.Title = "Editor";
	props.Width = 1280;
	props.Height = 720;
	props.Fullscreen = false;
	props.FullScreenType = BORDERLESS;
	props.ScreenMaximized = false;
	return new EditorApp(props);
}
