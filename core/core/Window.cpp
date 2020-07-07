#include "Window.h"

#include <iostream>

#include "GLFW/glfw3.h"

#ifdef LSIS_PLATFORM_WIN
#define GLFW_EXPOSE_NATIVE_WIN32
#endif
#include "GLFW/glfw3native.h"

#include "../platform/windows/Win32Util.h"

#include "glad/glad.h"

namespace LSIS {

	std::ostream& operator<<(std::ostream& os, glm::uvec2 vec) {
		return os << "[" << vec.x << "," << vec.y << "]";
	}

	std::ostream& operator<<(std::ostream& os, glm::vec2 vec) {
		return os << "[" << vec.x << "," << vec.y << "]";
	}

	const char* vertex_path = "kernels/window.vert";
	const char* fragment_path = "kernels/window.frag";


	Window::Window()
		: m_title("dafualt"), m_size(720, 512), m_native_window(nullptr)
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
		glViewport(0, 0, size[0], size[1]);
	}

	void Window::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void Window::SetCursorPos(glm::vec2& pos)
	{
		cursor_last_pos = pos;
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

	const char* Window::GetTitle() const
	{
		return m_title;
	}

	const glm::uvec2 Window::GetSize() const
	{
		return m_size;
	}

	const glm::vec2 Window::GetCursorPos() const
	{
		return cursor_last_pos;
	}

	void Window::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Window::Update()
	{

		m_shader->Bind();
		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	bool Window::IsCloseRequested() const
	{
		if (glfwWindowShouldClose(m_native_window))
			return true;

		if (glfwGetKey(m_native_window, GLFW_KEY_ESCAPE))
			return true;

		return false;
	}

	void Window::ReloadShaders()
	{
		// replace shader with a new version
		m_shader = Shader::Create(vertex_path, fragment_path);
		std::cout << "Reloaded Window Shaders\n";
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
		InitGLFW();
		InitGL();
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

		glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

		glm::uvec2 window_size = { (int)((float)m_size.x * xscale), (int)((float)m_size.y * yscale) };

		m_native_window = glfwCreateWindow(window_size.x, window_size.y, m_title, nullptr, nullptr);
		glfwMakeContextCurrent(m_native_window);

		CenterWindow();
		glfwShowWindow(m_native_window);

		glfwSetWindowUserPointer(m_native_window, this);

		{
			double x, y;
			glfwGetCursorPos(m_native_window, &x, &y);
			cursor_last_pos = { x,y };
		}

		glfwSetWindowSizeCallback(m_native_window, [](GLFWwindow* native_window, int width, int height) {
			Window& window = *(Window*)glfwGetWindowUserPointer(native_window);

			glViewport(0, 0, width, height);
		});

		glfwSetWindowMaximizeCallback(m_native_window, [](GLFWwindow* native_window, int code) {
			std::cout << "Maximize!\n";
		});

		glfwSetKeyCallback(m_native_window, [](GLFWwindow* native_window, int key, int scancode, int action, int mods) {
			Window& window = *(Window*)glfwGetWindowUserPointer(native_window);

			/*
#ifdef LSIS_PALTFORM_WIN
			if (key == GLFW_KEY_LEFT_SUPER) {
				HWND hwnd = glfwGetWin32Window(native_window);
				if (action == GLFW_PRESS) {
					LONG style = GetWindowLongA(hwnd, GWL_STYLE);
					style = style | WS_SIZEBOX;
					SetWindowLongA(hwnd, GWL_STYLE, style);
				}
				else if (action == GLFW_RELEASE) {
					LONG style = GetWindowLongA(hwnd, GWL_STYLE);
					style = style & (~WS_SIZEBOX);
					SetWindowLongA(hwnd, GWL_STYLE, style);
				}
			}
#endif
			*/


			if (action == GLFW_PRESS) {
				switch (key)
				{
				case GLFW_KEY_SPACE:
					window.ReloadShaders();
					break;
				case GLFW_KEY_C:
					window.CenterWindow();
					break;
				case GLFW_KEY_M:
					window.Maximize();
					break;
				case GLFW_KEY_LEFT_SUPER:
					std::cout << "Super!!!\n";
					break;
				}
			}
		});

		glfwSetCursorPosCallback(m_native_window, [](GLFWwindow* native_window, double x, double y) {
			Window& window = *(Window*)glfwGetWindowUserPointer(native_window);

			glm::vec2 pos = { x,y };
			glm::vec2 diff = pos - window.GetCursorPos();

			/*
			if (glfwGetMouseButton(native_window, GLFW_MOUSE_BUTTON_1) == GLFW_PRESS) {
				int win_x, win_y;
				glfwGetWindowPos(native_window, &win_x, &win_y);
				glfwSetWindowPos(native_window, win_x + diff.x, win_y + diff.y);
				pos -= diff;
				glfwSetCursorPos(native_window, pos.x, pos.y);
			}
			*/

			window.SetCursorPos(pos);
		});

	}

	void Window::InitGL()
	{
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		glClearColor(1.0, 0.0, 1.0, 1.0);
		glViewport(0, 0, m_size[0], m_size[1]);

		m_shader = Shader::Create(vertex_path, fragment_path);

		const float x = 0.5f;
		float vertices[] = {
			-x, -x, 0.0f, 0.0f, 0.0f,
			 x, -x, 0.0f, 1.0f, 0.0f,
			 x,  x, 0.0f, 1.0f, 1.0f,
			-x,  x, 0.0f, 0.0f, 1.0f
		};

		glGenBuffers(1, &m_vbo);
		glGenVertexArrays(1, &m_vao);

		glBindVertexArray(m_vao);
		glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(GL_FLOAT)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
	}

}