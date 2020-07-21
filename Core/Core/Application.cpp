#include "pch.h"
#include "Application.h"

#include "Core.h"
#include "Input/KeyCodes.h"
#include "Input/Input.h"

#include "CL/cl.hpp"
#include "CL/cl.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"

#include "Scene/Camera.h"

#include "Mesh/MeshLoader.h"

#include "Graphics/RenderCommand.h"

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
		m_scene = std::make_unique<Scene>();

		m_cam = std::make_shared<Camera>();
		m_scene->SetCamera(m_cam);

		auto flat = Shader::Create("../Assets/Shaders/flat.vert", "../Assets/Shaders/flat.frag");
		auto m1 = std::make_shared<Material>(flat, glm::vec4(1, 0, 0, 1));
		auto m2 = std::make_shared<Material>(flat, glm::vec4(0, 1, 0, 1));
		auto m3 = std::make_shared<Material>(flat, glm::vec4(0, 0, 1, 1));
		auto m4 = std::make_shared<Material>(flat, glm::vec4(1, 1, 1, 1));

		auto square = MeshLoader::CreateCube(0.5f);
		//auto bunny = MeshLoader::LoadFromOBJ("../models/bunny.obj");

		m_scene->LoadObject("../Assets/Models/bunny.obj", m1, Transform({ -1,0,-1 }));
		m_scene->LoadObject("../Assets/Models/cube.obj", m2, Transform({ 1,0,1 }));
		m_scene->LoadObject("../Assets/Models/cube.obj", m3, Transform({ -1,0,1 }));
		m_scene->LoadObject("../Assets/Models/cube.obj", m4, Transform({ 1,0,-1 }));

		m_scene->AddObject(std::make_shared<Object>(MeshLoader::CreateRect({ 10.0,10.0 }), m4, Transform({ 0,0,0 }, { -3.14 / 2.0,0,0 })));

		m_scene->AddLight(std::make_shared<Light>(glm::vec3(4, 4, 4), glm::vec3(1, 1, 1)));
		m_scene->AddLight(std::make_shared<Light>(glm::vec3(0, 5, 0), glm::vec3(1, 1, 1)));
		m_scene->AddLight(std::make_shared<Light>(glm::vec3(2, 3, -5), glm::vec3(0, 0, 1)));
		m_scene->AddLight(std::make_shared<Light>(glm::vec3(-5, 4, -2), glm::vec3(1, 1, 1)));
	}

	void Application::CreateWindow() {
		m_window = std::make_unique<Window>();
		m_window->SetEventCallback(BIND_EVENT_FN(Application::OnEvent));
		m_window->SetClearColor({ 0.05,0.05,0.05,1.0 });
	}

	void Application::CreateCLContext() {
		clGetGLContextInfoKHR_fn pclGetGLContextInfoKHR = (clGetGLContextInfoKHR_fn)clGetExtensionFunctionAddressForPlatform(m_platform(), "clGetGLContextInfoKHR");

		// Find a prefered platform and device
		m_platform = Compute::GetPreferedPlatform(prefered_platforms);
		m_device = Compute::GetPreferedDevice(m_platform, prefered_devices);

		std::cout << "Platform: " << Compute::GetName(m_platform) << std::endl;
		std::cout << "Device: " << Compute::GetName(m_device) << std::endl;

		// Create Context
		auto properties = m_window->GetCLProperties(m_platform());
		m_context = Compute::CreateContext(properties, m_device);
		m_queue = Compute::CreateCommandQueue(m_context, m_device);
	}

	void Application::Init()
	{
		CreateWindow();
		CreateCLContext();

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
	}

	void Application::Run()
	{
		std::cout << "Running\n";

		std::cout << "Num Objects: " << m_scene->GetNumObjects() << std::endl;
		std::cout << "Num Lights:  " << m_scene->GetNumLights() << std::endl;

		//window.ReloadShaders();
		double last = glfwGetTime();
		double time;

		while (!m_window->IsCloseRequested()) {
			time = glfwGetTime();
			float delta = (float)(time - last);
			last = time;

			m_window->Clear();

			Input::Update(delta);
			UpdateCam();

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


	const cl::Context& Application::GetContext()
	{
		return m_context;
	}

	const cl::Device& Application::GetDevice()
	{
		return m_device;
	}

	const cl::CommandQueue& Application::GetCommandQueue()
	{
		return m_queue;
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

	Application::Application()
	{
		Init();
	}

	Application::~Application()
	{
	}

}