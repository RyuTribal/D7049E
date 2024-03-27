#include "pch.h"
#include "LinuxWindow.h"

#include "Renderer/RenderContext.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"
#include "glad/gl.h"

namespace Engine
{
	static bool s_GLFWInitialized = false;

	static void GLFWErrorCallback(int error, const char *description)
	{
		CORE_ERROR("GLFW Error ({0}): {1}", error, description);
	}

	Window *Window::Create(const WindowProps &props)
	{
		return new LinuxWindow(props);
	}

	LinuxWindow::LinuxWindow(const WindowProps &props)
	{
		Init(props);
	}

	LinuxWindow::~LinuxWindow()
	{
		Shutdown();
	}

	void LinuxWindow::Init(const WindowProps &props)
	{
		m_Data.Title = props.Title;
		m_Data.Width = props.Width;
		m_Data.Height = props.Height;
		m_Data.VSync = false;

		CORE_INFO("Creating window  {0} ({1}, {2})", props.Title, props.Width, props.Height);

		if (!s_GLFWInitialized)
		{
			int success = glfwInit();
			CORE_ASSERT(success, "Could not initialize GLFW!");
			glfwSetErrorCallback(GLFWErrorCallback);
			s_GLFWInitialized = true;
		}

		m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
		m_Context = RenderContext::Create(m_Window);
		m_Context->Init();
		glfwSetWindowUserPointer(m_Window, &m_Data);
		glfwSwapInterval(m_Data.VSync);

		// Set GLFW callbacks
		glfwSetWindowSizeCallback(m_Window, [](GLFWwindow *window, int width, int height)
								  {
									  WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
									  data.Width = width;
									  data.Height = height;
									  WindowResizeEvent event(width, height);
									  data.EventCallback(event);
								  });
		glfwSetWindowCloseCallback(m_Window, [](GLFWwindow *window)
								   {
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.EventCallback(event); });

		glfwSetKeyCallback(m_Window, [](GLFWwindow *window, int key, int scancode, int action, int mods)
						   {
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key, 0);
					data.EventCallback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.EventCallback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, 1);
					data.EventCallback(event);
					break;
				}
				} });

		glfwSetCharCallback(m_Window, [](GLFWwindow *window, unsigned int keycode)
							{
								WindowData &data = *(WindowData *)glfwGetWindowUserPointer(window);
								KeyTypedEvent event(keycode);
								data.EventCallback(event);
							});

		glfwSetMouseButtonCallback(m_Window, [](GLFWwindow *window, int button, int action, int mods)
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
				} });

		glfwSetScrollCallback(m_Window, [](GLFWwindow *window, double x_offset, double y_offset)
							  {
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseScrolledEvent event((float)x_offset, (float)y_offset);
				data.EventCallback(event); });

		glfwSetCursorPosCallback(m_Window, [](GLFWwindow *window, double xPos, double yPos)
								 {
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseMovedEvent event((float)xPos, (float)yPos);
				data.EventCallback(event); });
	}

	void LinuxWindow::Shutdown()
	{
		glfwDestroyWindow(m_Window);
	}

	void LinuxWindow::OnUpdate()
	{
		glfwPollEvents();
		m_Context->SwapBuffers();
	}

}
