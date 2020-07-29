#include "pch.h"
#include "PixelViewer.h"

#include "Graphics/Shader.h"

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
		"	FragColor = vec4(texture(ourTexture, TexCoord).xyzw);\n"
		"}\n";

	PixelViewer::PixelViewer(uint32_t width, uint32_t height)
		: m_width(width), m_height(height), m_vao(0), m_vbo(0), m_texture(0)
	{
		//generate the texture ID
		glGenTextures(1, &m_texture);
		//binnding the texture
		glBindTexture(GL_TEXTURE_2D, m_texture);
		//regular sampler params
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		//need to set GL_NEAREST
		//(not GL_NEAREST_MIPMAP_* which would cause CL_INVALID_GL_OBJECT later)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, m_width, m_height, 0, GL_RGBA, GL_FLOAT, nullptr);
		glBindTexture(GL_TEXTURE_2D, 0);

		cl_int status = 0;
		m_shared_texture = clCreateFromGLTexture(Compute::GetContext()(), CL_MEM_READ_WRITE, GL_TEXTURE_2D, 0, m_texture, &status);
		CHECK(status);

		const float x = 1.0f;
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

		int  success;
		char infoLog[512];

		GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		Shader::CompileShader(vertexShaderSource, vertexShader);
		Shader::CompileShader(fragmentShaderSource, fragmentShader);

		m_shader = glCreateProgram();
		glAttachShader(m_shader, vertexShader);
		glAttachShader(m_shader, fragmentShader);
		glLinkProgram(m_shader);

		glGetProgramiv(m_shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(m_shader, 512, NULL, infoLog);
		}

		CompileKernels();
	}

	PixelViewer::~PixelViewer()
	{
		glDeleteVertexArrays(1, &m_vao);
		glDeleteBuffers(1, &m_vbo);
		glDeleteTextures(1, &m_texture);
	}

	void PixelViewer::SetResolution(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
	}

	void PixelViewer::UpdateTexture(const TypedBuffer<SHARED::Pixel>& pixels, uint32_t width, uint32_t height)
	{
		const cl::CommandQueue& queue = Compute::GetCommandQueue();
		clEnqueueAcquireGLObjects(queue(), 1, &m_shared_texture, 0, 0, 0);

		cl_int2 dim{};
		dim.x = width;
		dim.y = height;
		const int num_pixels = width * height;

		m_kernel.setArg(0, pixels.GetBuffer());
		m_kernel.setArg(1, sizeof(int2), &dim);
		clSetKernelArg(m_kernel(), 2, sizeof(m_shared_texture), &m_shared_texture);

		queue.enqueueNDRangeKernel(m_kernel, 0, cl::NDRange(num_pixels), cl::NullRange);

		clEnqueueReleaseGLObjects(queue(), 1, &m_shared_texture, 0, 0, NULL);
	}

	void PixelViewer::UpdateTexture(const glm::vec4* pixels, uint32_t width, uint32_t height)
	{
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, pixels);
	}

	void PixelViewer::Render()
	{
		glUseProgram(m_shader);
		glBindTexture(GL_TEXTURE_2D, m_texture);
		glBindVertexArray(m_vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
	}

	void PixelViewer::CompileKernels()
	{
		m_program = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "../Assets/Kernels/write_pixels.cl");
		m_kernel = Compute::CreateKernel(m_program, "write_pixels");
	}

}