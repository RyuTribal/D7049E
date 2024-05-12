#include "pch.h"
#include "WindowsWindow.h"

#include "Renderer/RenderContext.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "glad/gl.h"


namespace Engine
{
	static bool s_GLFWInitialized = false;

	static std::unordered_map<uint32_t, bool> s_KeyStates;

	static void GLFWErrorCallback(int error, const char* description)
	{
		HVE_CORE_ERROR_TAG("WindowsWindow", "GLFW Error ({0}): {1}", error, description);
	}

	Window* Window::Create(const WindowProps& props)
	{
		return new WindowsWindow(props);
	}

	WindowsWindow::WindowsWindow(const WindowProps& props)
	{
		Init(props);
	}

	WindowsWindow::~WindowsWindow()
	{
		Shutdown();
	}

	void WindowsWindow::SetTitle(std::string& new_title)
	{
		glfwSetWindowTitle(m_Window, new_title.c_str());
	}

	void WindowsWindow::SetFullScreen(bool fullscreen, FullscreenType type)
	{
		if (m_Data.Fullscreen == fullscreen) {
			return;
		}
		m_Data.Fullscreen = fullscreen;
		m_Data.FullscreenType = type;
		if (!fullscreen) {
			glfwSetWindowMonitor(m_Window, nullptr, XPos, YPos, PrevWidth, PrevHeight, 0);
			glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_TRUE);
			glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, GLFW_TRUE);
		}
		else {
			GLFWmonitor* monitor = glfwGetPrimaryMonitor();
			const GLFWvidmode* mode = glfwGetVideoMode(monitor);

			switch (type) {
			case FullscreenType::FULLSCREEN:

				glfwGetWindowPos(m_Window, &XPos, &YPos);
				glfwGetWindowSize(m_Window, &PrevWidth, &PrevHeight);
				glfwSetWindowMonitor(m_Window, monitor, 0, 0, mode->width, mode->height, mode->refreshRate);
				break;
			case FullscreenType::BORDERLESS:
				glfwGetWindowPos(m_Window, &XPos, &YPos);
				glfwGetWindowSize(m_Window, &PrevWidth, &PrevHeight);
				monitor = glfwGetPrimaryMonitor();
				mode = glfwGetVideoMode(monitor);
				if (!mode) return;
				glfwSetWindowAttrib(m_Window, GLFW_DECORATED, GLFW_FALSE); // Note to self: Dont be a dumbass and do this after setting window monitor (4 hours wasted you fucking asshole)
				glfwSetWindowAttrib(m_Window, GLFW_RESIZABLE, GLFW_FALSE);
				glfwSetWindowMonitor(m_Window, NULL, 0, 0, mode->width, mode->height, 0);
				break;
			}
		}
	}

	void WindowsWindow::SetMaximized(bool maximized)
	{
		if (maximized) {
			glfwMaximizeWindow(m_Window);
		}
	}

	bool WindowsWindow::IsKeyPressed(uint32_t key)
	{
		return s_KeyStates[key];
	}

	void WindowsWindow::SetKeyState(uint32_t key, bool state)
	{
		s_KeyStates[key] = state;
	}

	void WindowsWindow::ClearKeyStates()
	{
		s_KeyStates.clear();
	}

	void WindowsWindow::Init(const WindowProps& props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.VSync = props.VSync;
		m_Data.Fullscreen = props.Fullscreen;
		m_Data.FullscreenType = props.FullScreenType;
		m_Data.ScreenMaximized = props.ScreenMaximized;

		HVE_CORE_TRACE_TAG("Window", "Creating window  {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			HVE_CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		if (!props.Resizable)
		{
			glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		m_Context = RenderContext::Create(m_Window);
		m_Context->Init();
		glfwSetWindowUserPointer(m_Window, &m_Data);
		glfwSwapInterval(m_Data.VSync ? 1 : 0);
		

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, OnSizeChange);
		glfwSetWindowMaximizeCallback(m_Window, WindowsWindow::OnMaximize);
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
		{
			WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
			WindowCloseEvent event;
			data.EventCallback(event);
		});

		glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				switch (action)
				{
				case GLFW_PRESS:
				{
					s_KeyStates[key] = true;
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					s_KeyStates[key] = false;
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					s_KeyStates[key] = true;
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
				}
			});

		glfwSetCharCallback(m_Window, [](GLFWwindow* window, unsigned int keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent event(keycode);
				data.EventCallback(event);

			});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
					case GLFW_PRESS:
					{
						MouseButtonPressedEvent event(button);
						data.EventCallback(event);
						break;
					}
					case GLFW_RELEASE:
					{
						MouseButtonReleasedEvent event(button);
						data.EventCallback(event);
						break;
					}
				}
			});

		glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double x_offset, double y_offset)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseScrolledEvent event((float)x_offset, (float)y_offset);
				data.EventCallback(event);
			});

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event);
			});
	}

	void WindowsWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

	void WindowsWindow::OnMaximize(GLFWwindow* window, int maximized)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.ScreenMaximized = maximized;
	}

	void WindowsWindow::OnSizeChange(GLFWwindow* window, int width, int height)
	{
		WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
		data.Width = width;
		data.Height = height;
		WindowResizeEvent event(width, height);
		data.EventCallback(event);
	}

	bool WindowsWindow::IsFullScreen()
	{
		return glfwGetWindowMonitor(m_Window) != nullptr;
	}

	void WindowsWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
	}


}
