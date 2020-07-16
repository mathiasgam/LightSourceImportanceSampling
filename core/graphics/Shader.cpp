#include "pch.h"
#include "Shader.h"

#include <iostream>
#include <fstream>
#include <future>

#include "glad/glad.h"
#include "gtc/type_ptr.hpp"

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

	void Shader::UploadUniformInt(const std::string& name, const int vec)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform1iv(location, 1, &vec);
	}

	void Shader::UploadUniformfloat1(const std::string& name, const float vec)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform1fv(location, 1, &vec);
	}

	void Shader::UploadUniformfloat2(const std::string& name, const glm::vec2& vec)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform2fv(location, 1, glm::value_ptr(vec));
	}

	void Shader::UploadUniformfloat3(const std::string& name, const glm::vec3& vec)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform3fv(location, 1, glm::value_ptr(vec));
	}

	void Shader::UploadUniformfloat4(const std::string& name, const glm::vec4& vec)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniform4fv(location, 1, glm::value_ptr(vec));
	}

	void Shader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_program, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
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

	std::shared_ptr<Shader> Shader::Create(const char* vertex_path, const char* fragment_path)
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
		return std::make_shared<Shader>(program_id);
	}


}