#pragma once

#include <cinttypes>
#include <memory>
#include <functional>

#include "glm.hpp"
#include "CL/cl.h"

#include "Graphics/Shader.h"
#include "Event/Event.h"

namespace {
	struct GLFWwindow;
}

namespace LSIS {

	class Window
	{
		using EventCallbackFunc = std::function<void(const LSIS::Event&)>;

		struct WindowData {
			std::string Title;
			unsigned int Width, Height;
			EventCallbackFunc EventCallback;
		};

	public:

		Window(WindowData data = { "App", 512, 512, nullptr });
		virtual ~Window();

		void SetTitle(const char*);
		void SetSize(const glm::uvec2& size);
		void SetClearColor(const glm::vec4& color);
		void SetEventCallback(const EventCallbackFunc& callback);

		void CenterWindow();
		void Maximize();

		std::string GetTitle() const;
		const glm::uvec2 GetSize() const;
		cl_context_properties* GetCLProperties(const cl_platform_id platform_id) const;

		void Clear();
		void Update();
		bool IsCloseRequested() const;

		void SwapBuffers();
		void PollEvents();
		void WaitForEvent();

		void Show();
		void Hide();

	private:

		void Init();
		void SetWindowCallbacks();
		void SetMouseCallbacks();
		void SetKeyCallbacks();

	private:
		GLFWwindow* m_native_window = nullptr;
		WindowData m_Data;
	};


}