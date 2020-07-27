#pragma once

#include <string>
#include <memory>

#include "glm.hpp"

namespace LSIS {

	enum class ShaderType {
		Vertex,
		Fragment
	};

	class Shader {
	public:
		Shader(unsigned int program_id);
		virtual ~Shader();

		void Bind();
		void Unbind();

		void UploadUniformInt(const std::string& name, const int vec);
		void UploadUniformfloat1(const std::string& name, const float vec);
		void UploadUniformfloat2(const std::string& name, const glm::vec2& vec);
		void UploadUniformfloat3(const std::string& name, const glm::vec3& vec);
		void UploadUniformfloat4(const std::string& name, const glm::vec4& vec);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

		static std::shared_ptr<Shader> Create(const char* vertex_source, const char* fragment_source);
		static void CompileShader(const char* source, unsigned int shader_id);

	private:
		unsigned int m_program;
	};

}