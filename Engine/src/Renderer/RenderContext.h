#pragma once

struct GLFWwindow;

namespace Engine {

	class RenderContext
	{
	public:
		RenderContext(GLFWwindow* windowHandle);

		void Init();
		void SwapBuffers();
		void SetVSync(bool vsync);

		static std::unique_ptr<RenderContext> Create(void* window);

	private:
		GLFWwindow* m_WindowHandle;
	};
}