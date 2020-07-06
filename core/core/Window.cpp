#include "Window.h"

#include <iostream>

#include "GLFW/glfw3.h"
#include "glad/glad.h"

namespace LSIS {

	

	Window::Window()
		: m_title("dafualt"), m_size(720,512), m_native_window(nullptr)
	{
		Init();
	}

	Window::Window(const char* title, const glm::uvec2& size)
		: m_title(title), m_size(size), m_native_window(nullptr)
	{
		Init();
	}

	Window::~Window()
	{
	}

	void Window::SetTitle(const char* title)
	{
		m_title = title;
	}

	void Window::SetSize(const glm::uvec2& size)
	{
		m_size = size;
	}

	void Window::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	const char* Window::GetTitle() const
	{
		return m_title;
	}

	const glm::uvec2 Window::GetSize() const
	{
		return m_size;
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	bool Window::IsCloseRequested() const
	{
		if (glfwWindowShouldClose(m_native_window))
			return true;

		if (glfwGetKey(m_native_window, GLFW_KEY_ESCAPE))
			return true;

		return false;
	}

	void Window::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Window::SwapBuffers()
	{
		glfwSwapBuffers(m_native_window);
	}

	void Window::Show()
	{
	}

	void Window::Hide()
	{
	}

	void Window::Init()
	{
		InitGLFW();
		InitGLAD();
	}

	void Window::InitGLFW()
	{
		if (glfwInit() != GLFW_TRUE) {
			std::cout << "Failed to initialize GLFW\n";
			exit(-1);
		}

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();

		float xscale, yscale;
		glfwGetMonitorContentScale(monitor, &xscale, &yscale);

		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

		m_native_window = glfwCreateWindow((int)((float)m_size.x * xscale), (int)((float)m_size.y * yscale), m_title, nullptr, nullptr);
		glfwMakeContextCurrent(m_native_window);
	}

	void Window::InitGLAD()
	{
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		glClearColor(1.0, 0.0, 1.0, 1.0);
	}

}