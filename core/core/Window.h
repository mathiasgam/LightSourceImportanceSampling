#pragma once

#include <cinttypes>

#include "glm.hpp"

namespace {
	class GLFWwindow;
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

		void PollEvents();
		bool IsCloseRequested() const;

		void Clear();
		void SwapBuffers();

		void Show();
		void Hide();

	private:

		void Init();
		void InitGLFW();
		void InitGLAD();

	private:
		const char* m_title;
		glm::uvec2 m_size;

		GLFWwindow* m_native_window = nullptr;

	};


}