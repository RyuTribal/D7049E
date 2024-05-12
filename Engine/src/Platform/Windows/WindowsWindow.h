#pragma once
#include "Core/Window.h"

#include <GLFW/glfw3.h>

namespace Engine
{
	class WindowsWindow : public Window
	{
	public:
		WindowsWindow(const WindowProps &props);
		virtual ~WindowsWindow();

		void OnUpdate() override;

		inline unsigned int GetWidth() const override { return m_Data.Width; }
		inline unsigned int GetHeight() const override { return m_Data.Height; }
		inline bool GetFullScreen() const override { return m_Data.Fullscreen; }
		inline FullscreenType GetFullScreenType() const override { return m_Data.FullscreenType; }
		inline bool GetMaximized() const override { return m_Data.ScreenMaximized; }
		inline bool GetVSync() const override { return m_Data.VSync; }
		inline std::string& GetTitle() override { return m_Data.Title; }
		void SetTitle(std::string& new_title) override;

		inline void SetEventCallback(const EventCallbackFn &callback) override { m_Data.EventCallback = callback; }

		inline void *GetNativeWindow() const override { return m_Window; }

		void SetFullScreen(bool fullscreen, FullscreenType type) override;
		void SetMaximized(bool maximized) override;

		bool IsKeyPressed(uint32_t key) override;
		void SetKeyState(uint32_t key, bool state) override;
		void ClearKeyStates() override;

	private:
		virtual void Init(const WindowProps &props);
		virtual void Shutdown();
		static void OnMaximize(GLFWwindow* window, int maximized);
		static void OnSizeChange(GLFWwindow* window, int width, int height);
		bool IsFullScreen();
		GLFWwindow *m_Window;

		struct WindowData
		{

			std::string Title;
			unsigned int Width, Height;
			bool VSync;
			bool ScreenMaximized;
			bool Fullscreen;
			FullscreenType FullscreenType;
			EventCallbackFn EventCallback;
		};

		WindowData m_Data;
		int XPos = 0, YPos = 0, PrevWidth, PrevHeight;
	};
}
