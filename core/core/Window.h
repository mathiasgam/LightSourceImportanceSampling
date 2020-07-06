#pragma once

#include <cinttypes>
#include <memory>

#include "glm.hpp"
#include "CL/cl.h"

#include "../graphics/Shader.h"

namespace {
	struct GLFWwindow;
}

namespace LSIS {

	class Window
	{
	public:
		Window();
		Window(const char* title, const glm::uvec2& size);
		virtual ~Window();

		void SetTitle(const char*);
		void SetSize(const glm::uvec2& size);
		void SetClearColor(const glm::vec4& color);

		const char* GetTitle() const;
		const glm::uvec2 GetSize() const;

		void Clear();
		void Update();
		bool IsCloseRequested() const;

		void ReloadShaders();

		void SwapBuffers();
		void PollEvents();
		void WaitForEvent();

		void Show();
		void Hide();

	private:

		void Init();
		void InitGLFW();
		void InitGL();

	private:
		const char* m_title;
		glm::uvec2 m_size;

		GLFWwindow* m_native_window = nullptr;

		std::unique_ptr<Shader> m_shader;

		unsigned int m_vbo;
		unsigned int m_vao;

		
	};


}