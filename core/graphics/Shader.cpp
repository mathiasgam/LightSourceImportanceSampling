#include "Shader.h"

#include <iostream>
#include <fstream>
#include <future>

#include "glad/glad.h"

namespace LSIS {


	Shader::Shader(unsigned int program_id)
		: m_program(program_id)
	{
	}

	Shader::~Shader()
	{
		glDeleteProgram(m_program);
	}

	void Shader::Bind()
	{
		glUseProgram(m_program);
	}

	void Shader::Unbind()
	{
		glUseProgram(0);
	}

	static void ReadFile(const std::string& filepath, std::string* result)
	{
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in) {
			in.seekg(0, std::ios::end);
			result->resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(result->data(), result->size());
			in.close();
		}
		else {
			std::cout << "Failed to read file!\n";
		}
	}

	static void CompileShader(const char* source, unsigned int shader_id) {
		int  success;
		char infoLog[512];
		glShaderSource(shader_id, 1, &source, nullptr);
		glCompileShader(shader_id);
		glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader_id, 512, nullptr, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

	}

	std::unique_ptr<Shader> Shader::Create(const char* vertex_path, const char* fragment_path)
	{
		int  success;
		char infoLog[512];

		std::future<void> futures[2];

		std::string vertex_source;
		std::string fragment_source;

		futures[0] = std::async(std::launch::async, ReadFile, vertex_path, &vertex_source);
		futures[1] = std::async(std::launch::async, ReadFile, fragment_path, &fragment_source);

		unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

		futures[0].get();
		futures[1].get();

		CompileShader(vertex_source.c_str(), vertexShader);
		CompileShader(fragment_source.c_str(), fragmentShader);


		unsigned int program_id = glCreateProgram();
		glAttachShader(program_id, vertexShader);
		glAttachShader(program_id, fragmentShader);
		glLinkProgram(program_id);

		glGetProgramiv(program_id, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(program_id, 512, NULL, infoLog);
		}
		return std::make_unique<Shader>(program_id);
	}


}