#include "Window.h"

#include <iostream>

#include "GLFW/glfw3.h"
#include "glad/glad.h"

namespace LSIS {

	const char* vertexShaderSource =
		"#version 330 core\n"
		"layout (location = 0) in vec3 pos;\n"
		"layout (location = 1) in vec2 tex;\n"
		"out vec2 TexCoord;\n"
		"void main()\n"
		"{\n"
		"	TexCoord = tex;\n"
		"   gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n"
		"}\0";

	const char* fragmentShaderSource =
		"#version 330 core\n"
		"out vec4 FragColor;\n"
		"in vec2 TexCoord;\n"
		"uniform sampler2D ourTexture;\n"
		"void main()\n"
		"{\n"
		"	//FragColor = texture(ourTexture, TexCoord);\n"
		"	FragColor = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0); \n"
		"}\n";

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
		glViewport(0, 0, size[0], size[1]);
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

	void Window::Clear()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void Window::Update()
	{
		glfwPollEvents();

		glUseProgram(m_program);
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

		glfwWindowHint(GLFW_VISIBLE, GLFW_TRUE);
		glfwWindowHint(GLFW_DECORATED, GLFW_TRUE);
		glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE);

		m_native_window = glfwCreateWindow((int)((float)m_size.x * xscale), (int)((float)m_size.y * yscale), m_title, nullptr, nullptr);
		glfwMakeContextCurrent(m_native_window);
	}

	void Window::InitGL()
	{
		int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
		glClearColor(1.0, 0.0, 1.0, 1.0);
		glViewport(0, 0, m_size[0], m_size[1]);

		int  success;
		char infoLog[512];

		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
		glCompileShader(vertexShader);
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
		glCompileShader(fragmentShader);
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		m_program = glCreateProgram();
		glAttachShader(m_program, vertexShader);
		glAttachShader(m_program, fragmentShader);
		glLinkProgram(m_program);

		glGetProgramiv(m_program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(m_program, 512, NULL, infoLog);
		}

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