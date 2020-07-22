#pragma once

#include "Layer.h"

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
		
		void Init();
		void Destroy();
		void Run();

		void AddLayer(Ref<Layer> layer);

		const cl::Context& GetContext();
		const cl::Device& GetDevice();
		const cl::CommandQueue& GetCommandQueue();
		const Ref<Scene> GetScene() const { return m_scene; }

		void OnEvent(const Event& e);

		void OnWindowResizedEvent(const WindowResizeEvent& e);

		static Application* Get();

	private:
		Application();
		virtual ~Application();

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
		Ref<Scene> m_scene;
		Ref<Camera> m_cam;

		std::vector<Ref<Layer>> m_layers{};
	};

}