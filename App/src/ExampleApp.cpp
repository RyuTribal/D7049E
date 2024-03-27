#include <Engine.h>
#include <Core/EntryPoint.h>
#include "TestLayer.h"

class ExampleApp : public Engine::Application
{
public:
	ExampleApp(Engine::WindowProps props) : Application(props)
	{
	    auto [camera_entity, scene] = Engine::Scene::CreateScene("Test scene");
		entities[camera_entity->GetID()] = camera_entity;
	    Engine::Renderer::Get()->SetBackgroundColor(0, 0, 0); //You can change the color here to see that it updates
		PushLayer(new TestApp::TestLayer(scene));
	}

	~ExampleApp()
	{

	}

	std::unordered_map <Engine::UUID, Engine::Ref<Engine::Entity>> entities{}; //To make sure the entities are not garbage collected
};

Engine::Application* Engine::CreateApplication()
{
	WindowProps props{};
	props.Title = "Example App";
	props.Width = 1280;
	props.Height = 720;
	return new ExampleApp(props);
}
