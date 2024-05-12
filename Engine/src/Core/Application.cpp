#include "pch.h"
#include "Core/Base.h"
#include "Application.h"
#include "Script/ScriptEngine.h"

#include "Renderer/Renderer.h"
#include "Physics/PhysicsEngine.h"
#include "Sound/SoundEngine.h"


namespace Engine
{
	Application* Application::s_Instance = nullptr;

	Application::Application(WindowProps props, ApplicationProps app_props) : m_AppProps(app_props)
	{
		HVE_CORE_ASSERT(!s_Instance, "Application already exists!");
		s_Instance = this;
		m_Window = std::unique_ptr<Window>(Window::Create(props));
		m_Window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		Renderer::CreateRenderer();
		m_ImGuiLayer = new ImGuiLayer();
		PushOverlay(m_ImGuiLayer);

		SoundEngine::Init();

		if (!m_AppProps.NoScripting)
		{
			ScriptEngine::Init();
			PhysicsEngine::Get()->Init(10);
		}
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<WindowCloseEvent>(BIND_EVENT_FN(Application::OnWindowClosed));
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(Application::OnWindowResize));

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnEvent(event);
			if (event.Handled)
				break;
		}
	}

	void Application::Close()
	{
		m_Running = false;
		if (!m_AppProps.NoScripting)
		{
			ScriptEngine::Shutdown();
			PhysicsEngine::Get()->Shutdown();
		}

		SoundEngine::Shutdown();

		for (auto it = m_LayerStack.end(); it != m_LayerStack.begin();)
		{
			(*--it)->OnDetach();
		}
	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.PushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* overlay)
	{
		m_LayerStack.PushLayer(overlay);
		overlay->OnAttach();
	}

	Application::~Application()
	{
	}

	void Application::run()
	{
		m_Window->SetMaximized(m_Window->GetMaximized());
		m_Window->SetFullScreen(m_Window->GetFullScreen(), m_Window->GetFullScreenType());

		auto last_frame = std::chrono::high_resolution_clock::now();
		//PhysicsEngine::tmpRunner();
		while (m_Running)
		{
			auto newTime = std::chrono::high_resolution_clock::now();
			float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - last_frame).count();
			last_frame = newTime;
			double currentTime = std::chrono::duration<double>(newTime.time_since_epoch()).count();
			m_FrameData.DeltaTime = frameTime;
			Renderer::Get()->GetStats()->UpdateFPS(currentTime, frameTime);

			if (!m_Minimized) {

				for (Layer* layer : m_LayerStack)
				{
					layer->OnUpdate(frameTime);
				}


				m_ImGuiLayer->Begin();
				for (Layer* layer : m_LayerStack)
				{
					layer->OnImGuiRender();
				}
				m_ImGuiLayer->End();
			}

			m_Window->OnUpdate();
			HVE_PROFILE_MARK_FRAME;
		}
	}

	bool Application::OnWindowClosed(WindowCloseEvent& e)
	{
		m_Running = false;
		return false;
	}
	bool Application::OnWindowResize(WindowResizeEvent& e)
	{
		if (e.GetHeight() == 0 && e.GetWidth() == 0) {
			m_Minimized = true;
			return false;
		}
		m_Minimized = false;
		Renderer::Get()->SetViewport(e.GetHeight(), e.GetWidth());

		return false;
	}
}
