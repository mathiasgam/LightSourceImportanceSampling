#include "pch.h"
#include "Application.h"

#include "Core.h"
#include "Log.h"
#include "Input/KeyCodes.h"
#include "Input/Input.h"

#include "CL/cl.hpp"
#include "CL/cl.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"

#include "Scene/Camera.h"
#include "Scene/Material.h"
#include "Scene/Components.h"
#include "Scene/Entity.h"

#include "Mesh/MeshLoader.h"

#include "Graphics/RenderCommand.h"
#include "Graphics/PointRenderer.h"
#include "Graphics/Renderer2D.h"

#include "Compute/Compute.h"

#include <iostream>
#include <functional>

namespace LSIS {

	std::vector<std::string> prefered_platforms = {
		"NVIDIA CUDA"
	};

	std::vector<std::string> prefered_devices = {
		"GeForce RTX 2080",
		"GeForce GTX 1050"
	};

	void Application::LoadScene() {
		m_scene = std::make_shared<Scene>();

		m_cam = std::make_shared<Camera>();
		m_cam->SetPosition({ -3.0f,1.0f,3.0f });
		m_scene->SetCamera(m_cam);

		//auto flat = Shader::Create("../Assets/Shaders/flat.vert", "../Assets/Shaders/flat.frag");
		//auto m1 = std::make_shared<Material>(flat, glm::vec4(199 / 256.0f, 151 / 256.0f, 40 / 256.0f, 1.0f));

		//m_scene->LoadObject("../Assets/Models/San_Miguel/san-miguel.obj", m1, Transform({ 0,0,0 }));
		//m_scene->LoadObject("../Assets/Models/Helix.obj", m1, Transform({ 0.0f,0.0f,0.0f }));
		//m_scene->LoadObject("../Assets/Models/Buddha.obj", m1, Transform({ 0,0,0 }));
		//m_scene->LoadObject("../Assets/Models/Background.obj", m1, Transform({ 0,0,0 }));
	}

	void Application::CreateWindow() {
		Window::WindowData win_props{ "LSIS", 512, 512, BIND_EVENT_FN(Application::OnEvent) };
		m_window = std::make_unique<Window>(win_props);
		m_window->SetClearColor({ 0.05,0.05,0.05,1.0 });
	}

	void Application::CreateCLContext() {

		// Find a prefered platform and device
		auto platform = Compute::GetPreferedPlatform(prefered_platforms);
		auto device = Compute::GetPreferedDevice(platform, prefered_devices);

		clGetGLContextInfoKHR_fn pclGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddressForPlatform(platform(), "clGetGLContextInfoKHR");

		std::cout << "Platform: " << Compute::GetName(platform) << std::endl;
		std::cout << "Device: " << Compute::GetName(device) << std::endl;

		// Create Context
		auto properties = m_window->GetCLProperties(platform());
		auto context = Compute::CreateContext(properties, device);
		auto queue = Compute::CreateCommandQueue(context, device);

		Compute::SetPlatform(platform);
		Compute::SetDevice(device);
		Compute::SetContext(context);
		Compute::SetCommandQueue(queue);
	}

	void Application::Init()
	{
		CreateWindow();
		CreateCLContext();

		PointRenderer::Init();
		LineRenderer::Init();

		LoadScene();

		std::cout << "Application Initialized\n";
		Initialized = true;
	}

	void Application::Destroy()
	{
		std::cout << "Application Destroyed\n";
	}

	void Application::UpdateCam() {
		m_cam->SetPosition(Input::GetCameraPosition());
		m_cam->SetRotation(Input::GetcameraRotation());
		OnEvent(CameraUpdatedEvent(m_cam));
	}

	void Application::Run()
	{
		std::cout << "Running\n";

		std::cout << "Num Objects: " << m_scene->GetNumEntities() << std::endl;
		std::cout << "Num Lights:  " << m_scene->GetNumLights() << std::endl;

		//window.ReloadShaders();
		double last = glfwGetTime();
		double time;

		m_window->Show();

		while (!m_window->IsCloseRequested()) {
			time = glfwGetTime();
			float delta = (float)(time - last);
			last = time;

			m_window->Clear();

			Input::Update(delta);
			if (Input::HasCameraMoved()) {
				UpdateCam();
			}

			m_scene->Update();
			m_scene->Render();

			for (auto& layer : m_layers) {
				layer->OnUpdate(delta);
			}

			// Do rendering

			m_window->Update();
			m_window->SwapBuffers();
			m_window->PollEvents();
		}
	}

	void Application::AddLayer(Ref<Layer> layer)
	{
		m_layers.push_back(layer);
	}

	void Application::OnEvent(const Event& e)
	{
		EventType type = e.GetEventType();
		int event_flags = e.GetCategoryFlags();

		if (event_flags & EventCategory::EventCategoryInput) {
			Input::OnEvent(e);
		}
		if (event_flags & EventCategory::EventCategoryApplication) {
			if (type == EventType::WindowResize) {
				OnWindowResizedEvent((const WindowResizeEvent&)e);
			}
		}

		for (auto& layer : m_layers) {
			int layer_flags = layer->GetEventCategoriesFlags();
			if ((layer_flags & event_flags) != 0) {
				if (layer->OnEvent(e)) {
					break;
				}
			}
		}
	}

	void Application::OnWindowResizedEvent(const WindowResizeEvent& e)
	{
		m_cam->SetResolution({ e.GetWidth(), e.GetHeight() });
		RenderCommand::SetViewPort(0, 0, e.GetWidth(), e.GetHeight());
	}

	Application* Application::Get()
	{
		static Application app = Application();
		return &app;
	}

	Application::Application()
	{
		Init();
	}

	Application::~Application()
	{
	}

}