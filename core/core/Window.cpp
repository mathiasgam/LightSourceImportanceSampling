#include "Window.h"

#include <iostream>

#include "GLFW/glfw3.h"

#ifdef LSIS_PLATFORM_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "GLFW/glfw3native.h"

#include "../platform/windows/Win32Util.h"

#include "glad/glad.h"

#include "graphics/RenderCommand.h"

#include "event/ApplicationEvent.h"
#include "event/MouseEvent.h"
#include "event/KeyEvent.h"

namespace LSIS {

	std::ostream& operator<<(std::ostream& os, glm::uvec2 vec) {
		return os << "[" << vec.x << "," << vec.y << "]";
	}

	std::ostream& operator<<(std::ostream& os, glm::vec2 vec) {
		return os << "[" << vec.x << "," << vec.y << "]";
	}

	const char* vertex_path = "kernels/window.vert";
	const char* fragment_path = "kernels/window.frag";


	Window::Window(WindowData data)
	{
		m_Data = data;
		m_native_window = nullptr;

		Init();
		SetWindowCallbacks();
		SetMouseCallbacks();
		SetKeyCallbacks();
	}

	Window::~Window()
	{
	}

	void Window::SetTitle(const char* title)
	{
		m_Data.Title = title;
		glfwSetWindowTitle(m_native_window, title);
	}

	void Window::SetSize(const glm::uvec2& size)
	{
		m_Data.Width = size.x;
		m_Data.Height = size.y;
		glViewport(0, 0, size[0], size[1]);
		glfwSetWindowSize(m_native_window, size.x, size.y);
	}

	void Window::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void Window::SetEventCallback(const EventCallbackFunc& callback)
	{
		m_Data.EventCallback = callback;
	}

	void Window::CenterWindow()
	{
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		int x, y, w, h, size_x, size_y;
		glfwGetMonitorWorkarea(monitor, &x, &y, &w, &h);
		glfwGetWindowSize(m_native_window, &size_x, &size_y);
		glfwSetWindowPos(m_native_window, x + (w - size_x) / 2, y + (h - size_y) / 2);
	}

	void Window::Maximize()
	{
		if (glfwGetWindowAttrib(m_native_window, GLFW_MAXIMIZED)) {
			glfwRestoreWindow(m_native_window);
		}
		else {
			glfwMaximizeWindow(m_native_window);
		}
		/*
		GLFWmonitor* monitor = glfwGetPrimaryMonitor();
		int x, y, w, h;
		glfwGetMonitorWorkarea(monitor, &x, &y, &w, &h);
		glfwSetWindowSize(m_native_window, w, h);
		glfwSetWindowPos(m_native_window, x, y);
		*/
	}

	std::string Window::GetTitle() const
	{
		return m_Data.Title;
	}

	const glm::uvec2 Window::GetSize() const
	{
		return glm::uvec2(m_Data.Width, m_Data.Height);
	}

	void Window::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void Window::Update()
	{

		/*
		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
		*/
	}

	bool Window::IsCloseRequested() const
	{
		if (glfwWindowShouldClose(m_native_window))
			return true;

		if (glfwGetKey(m_native_window, GLFW_KEY_ESCAPE))
			return true;

		return false;
	}

	void Window::SwapBuffers()
	{
		glfwSwapBuffers(m_native_window);
	}

	void Window::PollEvents()
	{
		glfwPollEvents();
	}

	void Window::WaitForEvent()
	{
		glfwWaitEvents();
	}

	void Window::Show()
	{
		glfwShowWindow(m_native_window);
	}

	void Window::Hide()
	{
		glfwHideWindow(m_native_window);
	}

	void Window::Init()
	{
		if (glfwInit() != GLFW_TRUE) {
			std::cout << "Failed to initialize GLFW\n";
			exit(-1);
		}

		GLFWmonitor* monitor = glfwGetPrimaryMonitor();

		float xscale, yscale;
		glfwGetMonitorContentScale(monitor, &xscale, &yscale);

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		glfwWindowHint(GLFW_SAMPLES, 4);

		glm::uvec2 window_size = { (int)((float)m_Data.Width * xscale), (int)((float)m_Data.Height * yscale) };

		m_native_window = glfwCreateWindow(window_size.x, window_size.y, m_Data.Title.c_str(), nullptr, nullptr);
		glfwSetWindowUserPointer(m_native_window, &m_Data);

		glfwMakeContextCurrent(m_native_window);

		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

		RenderCommand::Init();
		RenderCommand::SetViewPort(0, 0, m_Data.Width, m_Data.Height);

		CenterWindow();
		glfwShowWindow(m_native_window);

		glfwSetErrorCallback([](int code, const char* err) {
			std::cout << "Code: " << code << ": " << err << std::endl;
		});

	}

	void Window::SetWindowCallbacks()
	{
		glfwSetWindowSizeCallback(m_native_window, [](GLFWwindow* native_window, int width, int height) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(native_window);
			data->EventCallback(WindowResizeEvent(width, height));
		});

		glfwSetWindowMaximizeCallback(m_native_window, [](GLFWwindow* native_window, int code) {
			std::cout << "Maximize!\n";
		});
	}

	void Window::SetMouseCallbacks()
	{
		glfwSetCursorPosCallback(m_native_window, [](GLFWwindow* native_window, double x, double y) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(native_window);
			int mods = 0;
			mods = glfwGetMouseButton(native_window, GLFW_MOUSE_BUTTON_1) ? mods | BIT(0) : mods;
			mods = glfwGetMouseButton(native_window, GLFW_MOUSE_BUTTON_2) ? mods | BIT(1) : mods;

			data->EventCallback(MouseMovedEvent(x, y, mods));
		});

		glfwSetScrollCallback(m_native_window, [](GLFWwindow* native_window, double x, double y) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(native_window);
			data->EventCallback(MouseScrolledEvent(x, y));
		});

		glfwSetMouseButtonCallback(m_native_window, [](GLFWwindow* native_window, int key, int action, int mod) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(native_window);
			switch (action)
			{
			case GLFW_PRESS:
				data->EventCallback(MouseButtonPressedEvent(key, mod));
				break;
			case GLFW_RELEASE:
				data->EventCallback(MouseButtonReleasedEvent(key, mod));
				break;
			}
		});
	}

	void Window::SetKeyCallbacks()
	{
		glfwSetKeyCallback(m_native_window, [](GLFWwindow* native_window, int key, int scancode, int action, int mods) {
			WindowData* data = (WindowData*)glfwGetWindowUserPointer(native_window);
			switch (action)
			{
			case GLFW_PRESS:
				data->EventCallback(KeyPressedEvent(key, scancode, mods));
				break;
			case GLFW_REPEAT:
				data->EventCallback(KeyRepeatEvent(key, scancode, mods));
				break;
			case GLFW_RELEASE:
				data->EventCallback(KeyReleasedEvent(key));
				break;
			}
		});
	}

}