#pragma once
#include "LayerStack.h"
#include "Window.h"
#include "Events/ApplicationEvent.h"
#include "ImGui/ImGuiLayer.h"

namespace Engine
{
	struct FrameData
	{
		float DeltaTime = 0.0f;
	};

	struct ApplicationProps
	{
		bool NoScripting = false;
	};

	class Application
	{
	public:
		Application(WindowProps props = WindowProps(), ApplicationProps app_props = ApplicationProps());
		virtual ~Application();

		void run();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* overlay);

		void OnEvent(Event& event);
		void Close();
		FrameData& GetFrameData() { return m_FrameData; }
		static Application& Get() { return *s_Instance; }
		Window& GetWindow() { return *m_Window; }
		ApplicationProps& GetProps() { return m_AppProps; }

	private:
		bool OnWindowClosed(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		std::unique_ptr<Window> m_Window;
		ImGuiLayer* m_ImGuiLayer;
		bool m_Running = true;
		bool m_Minimized = false;
		LayerStack m_LayerStack;

		FrameData m_FrameData;

	private:
		static Application* s_Instance;
		ApplicationProps m_AppProps;
	};

	Application* CreateApplication(int argc, char** argv);
}
