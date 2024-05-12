#pragma once
#include "pch.h"
#include "Core/Base.h"
#include "Renderer/RenderContext.h"
#include "Events/Event.h"

namespace Engine {

	enum FullscreenType {
		FULLSCREEN,
		BORDERLESS
	};

	struct WindowProps
	{
		std::string Title;
		unsigned int Width;
		unsigned int Height;
		bool VSync;
		bool Fullscreen;
		bool ScreenMaximized;
		FullscreenType FullScreenType;
		bool Resizable = true;

		WindowProps(const std::string& title = "Engine",
			unsigned int width = 1280,
			unsigned int height = 720,
			bool vsync = true,
			bool fullscreen = false,
			bool maximized = false,
			FullscreenType fullscreen_type = BORDERLESS)
			: Title(title), Width(width), Height(height), VSync(vsync), Fullscreen(fullscreen), ScreenMaximized(maximized), FullScreenType(fullscreen_type)
		{
		}
	};
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		virtual ~Window() {};

		virtual void OnUpdate() = 0;

		virtual unsigned int GetWidth() const = 0;
		virtual unsigned int GetHeight() const = 0;
		virtual bool GetFullScreen() const = 0;
		virtual FullscreenType GetFullScreenType() const = 0;
		virtual bool GetMaximized() const = 0;
		virtual bool GetVSync() const = 0;
		virtual std::string& GetTitle() = 0;
		virtual void SetTitle(std::string& new_title) = 0;
		virtual void SetFullScreen(bool fullscreen, FullscreenType type) = 0;
		virtual void SetMaximized(bool maximized) = 0;

		virtual bool IsKeyPressed(uint32_t key) = 0;
		virtual void SetKeyState(uint32_t key, bool state) = 0;
		virtual void ClearKeyStates() = 0;

		virtual void SetEventCallback(const EventCallbackFn& callback) = 0;

		virtual void* GetNativeWindow() const = 0;

		static Window* Create(const WindowProps& props = WindowProps());

		RenderContext* GetContext() { return m_Context.get(); }

	protected:
		std::unique_ptr<RenderContext> m_Context;
	};
}
