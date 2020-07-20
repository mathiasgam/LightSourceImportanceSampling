#pragma once

#include "PathTracer.h"

#include "Window.h"
#include "Scene/Scene.h"

#include "glm.hpp"
#include "CL/cl.hpp"

#include <iostream>
#include <memory>

#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "Event/KeyEvent.h"

namespace LSIS {

	class Application {
	public:
		Application();
		virtual ~Application();

		void Init();
		void Destroy();
		void Run();

		const cl::Context& GetContext();
		const cl::Device& GetDevice();
		const cl::CommandQueue& GetCommandQueue();

		void OnEvent(const Event& e);

		void OnWindowResizedEvent(const WindowResizeEvent& e);

	private:
		void LoadScene();
		void CreateWindow();
		void CreateCLContext();
		void UpdateCam();

	private:
		bool Initialized = false;

		cl::Platform m_platform;
		cl::Device m_device;
		cl::Context m_context;
		cl::CommandQueue m_queue;

		Scope<Window> m_window;
		Scope<Scene> m_scene;
		Ref<Camera> m_cam;

		Scope<PathTracer> m_path_tracer;
	};

}