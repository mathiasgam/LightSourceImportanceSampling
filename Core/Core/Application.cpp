#include "pch.h"
#include "Application.h"

#include "Core.h"
#include "Input/KeyCodes.h"
#include "Input/Input.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "GLFW/glfw3.h"

#include "Scene/Camera.h"

#include "Mesh/MeshLoader.h"

#include "Graphics/RenderCommand.h"

#include "Compute/Platform.h"
#include "Compute/Device.h"
#include "Compute/Contex.h"
#include "Compute/CommandQueue.h"

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

	bool Initialized = false;

	std::unique_ptr<Window> m_window;
	std::unique_ptr<Scene> m_scene;
	std::shared_ptr<Camera> m_cam;

	cl_platform_id m_platform_id;
	cl_device_id m_device_id;
	cl_context m_context;
	cl_command_queue m_queue;

	void LoadScene() {
		m_scene = std::make_unique<Scene>();

		m_cam = std::make_shared<Camera>();
		m_scene->SetCamera(m_cam);

		auto flat = Shader::Create("Kernels/flat.vert", "Kernels/flat.frag");
		auto m1 = std::make_shared<Material>(flat, glm::vec4(1, 0, 0, 1));
		auto m2 = std::make_shared<Material>(flat, glm::vec4(0, 1, 0, 1));
		auto m3 = std::make_shared<Material>(flat, glm::vec4(0, 0, 1, 1));
		auto m4 = std::make_shared<Material>(flat, glm::vec4(1, 1, 1, 1));

		std::shared_ptr<MeshData> square = MeshLoader::CreateCube(0.5f);
		//auto bunny = MeshLoader::LoadFromOBJ("../models/bunny.obj");

		m_scene->LoadObject("../models/bunny.obj", m1, Transform({ -1,0,-1 }));
		m_scene->LoadObject("../models/cube.obj", m2, Transform({ 1,0,1 }));
		m_scene->LoadObject("../models/cube.obj", m3, Transform({ -1,0,1 }));
		m_scene->LoadObject("../models/cube.obj", m4, Transform({ 1,0,-1 }));

		m_scene->AddObject(std::make_shared<Object>(MeshLoader::CreateRect({ 10.0,10.0 }), m4, Transform({ 0,0,0 }, { -3.14 / 2.0,0,0 })));

		m_scene->AddLight(std::make_shared<Light>(glm::vec3(4, 4, 4), glm::vec3(1, 1, 1)));
		m_scene->AddLight(std::make_shared<Light>(glm::vec3(0, 5, 0), glm::vec3(1, 1, 1)));
		m_scene->AddLight(std::make_shared<Light>(glm::vec3(2, 3, -5), glm::vec3(0, 0, 1)));
		m_scene->AddLight(std::make_shared<Light>(glm::vec3(-5, 4, -2), glm::vec3(1, 1, 1)));
	}

	void CreateWindow() {
		m_window = std::make_unique<Window>();
		m_window->SetEventCallback(Application::OnEvent);
		m_window->SetClearColor({ 0.05,0.05,0.05,1.0 });
	}

	void CreateCLContext() {
		m_platform_id = Compute::Platform::GetPlatform(prefered_platforms);
		m_device_id = Compute::Device::GetDevice(m_platform_id, prefered_devices);

		std::cout << "Platform: " << Compute::Platform::GetName(m_platform_id) << std::endl;
		std::cout << "Device: " << Compute::Device::GetName(m_device_id) << std::endl;

		cl_int err;
		// Create Context
		m_context = Compute::Context::CreateContext(m_window->GetCLProperties(m_platform_id), m_device_id);
		m_queue = Compute::CommandQueue::CreateCommandQueue(m_context, m_device_id);
	}

	void Application::Init()
	{
		CreateWindow();
		CreateCLContext();

		LoadScene();

		std::cout << "Application Initialized\n";
		Initialized = true;
	}

	void UpdateCam() {
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
			double delta = time - last;
			last = time;

			m_window->Clear();

			Input::Update((float)delta);
			UpdateCam();

			m_scene->Update();
			m_scene->Render();

			// Do rendering

			m_window->Update();
			m_window->SwapBuffers();
			m_window->PollEvents();
		}
	}

	void Application::OnEvent(const Event& e)
	{
		EventType type = e.GetEventType();
		int category_flags = e.GetCategoryFlags();

		if (category_flags & EventCategory::EventCategoryInput) {
			Input::OnEvent(e);
		}
		if (category_flags & EventCategory::EventCategoryApplication) {
			if (type == EventType::WindowResize) {
				OnWindowResizedEvent((const WindowResizeEvent&)e);
			}
		}
	}

	void Application::OnWindowResizedEvent(const WindowResizeEvent& e)
	{
		RenderCommand::SetViewPort(0, 0, e.GetWidth(), e.GetHeight());
	}


}