#pragma once

#include <memory>

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

		static std::unique_ptr<Shader> Create(const char* vertex_source, const char* fragment_source);

	private:
		unsigned int m_program;
	};

}