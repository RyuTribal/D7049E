#include "pch.h"
#include "RenderContext.h"

#include <GLFW/glfw3.h>
#include <glad/gl.h>

namespace Engine
{
	RenderContext::RenderContext(GLFWwindow* windowHandle)
		: m_WindowHandle(windowHandle)
	{
		HVE_CORE_ASSERT(windowHandle, "Window handle is null!");
	}
	void RenderContext::Init()
	{
		HVE_PROFILE_FUNC();

		glfwMakeContextCurrent(m_WindowHandle);
		int status = gladLoadGL((GLADloadfunc)glfwGetProcAddress);
		HVE_CORE_ASSERT(status, "Failed to initialize Glad!");

		HVE_CORE_TRACE("OpenGL Info:");
		HVE_CORE_TRACE("Vendor: {0}", glGetString(GL_VENDOR));
		HVE_CORE_TRACE("Renderer: {0}", glGetString(GL_RENDERER));
		HVE_CORE_TRACE("Version: {0}", glGetString(GL_VERSION));

		glEnable(GL_DEBUG_OUTPUT);
	}
	void RenderContext::SwapBuffers()
	{
		glfwSwapBuffers(m_WindowHandle);
	}
	void RenderContext::SetVSync(bool vsync)
	{
		glfwSwapInterval(vsync ? 1 : 0);
	}
	std::unique_ptr<RenderContext> RenderContext::Create(void* window)
	{
		return std::make_unique<RenderContext>(std::forward<GLFWwindow*>(static_cast<GLFWwindow*>(window)));
	}
}
