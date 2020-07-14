#include "Application.h"

#include "Core.h"
#include "Input/KeyCodes.h"
#include "Input/Input.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "glad/glad.h"

#include "Scene/Camera.h"

#include "Mesh/MeshLoader.h"

#include "Compute/Contex.h"
#include "Compute/Platform.h"

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

	std::unique_ptr<Compute::Context> m_context;

	void Application::Init()
	{
		m_window = std::make_unique<Window>();
		m_window->SetEventCallback(Application::OnEvent);
		std::cout << "Created Application\n";

		cl_platform_id platform_id = Compute::Platform::GetPlatform(prefered_platforms);
		cl_device_id device_id = Compute::Device::GetDevice(platform_id, prefered_devices);

		std::cout << "Platform: " << Compute::Platform::GetName(platform_id) << std::endl;
		std::cout << "Device: " << Compute::Device::GetName(device_id) << std::endl;

		m_context = std::make_unique<Compute::Context>(m_window->GetCLProperties(platform_id), device_id);

		m_scene = std::make_unique<Scene>();

		m_cam = std::make_shared<Camera>();
		m_scene->SetCamera(m_cam);

		auto flat = Shader::Create("kernels/flat.vert", "kernels/flat.frag");
		auto m1 = std::make_shared<Material>(flat, glm::vec4(1, 0, 0, 1));
		auto m2 = std::make_shared<Material>(flat, glm::vec4(0, 1, 0, 1));
		auto m3 = std::make_shared<Material>(flat, glm::vec4(0, 0, 1, 1));
		auto m4 = std::make_shared<Material>(flat, glm::vec4(1, 1, 1, 1));

		std::shared_ptr<MeshData> square = MeshLoader::CreateCube(0.5f);
		//auto bunny = MeshLoader::LoadFromOBJ("../models/bunny.obj");

		m_scene->LoadObject("../models/bunny.obj", m1, Transform({ -0.5f,-1.0f ,0.0f }));
		m_scene->AddObject(std::make_shared<Object>(square, m2, Transform({ 0.5,0.5,0 })));
		m_scene->AddObject(std::make_shared<Object>(square, m3, Transform({ -0.5,0.5,0 })));
		m_scene->AddObject(std::make_shared<Object>(square, m4, Transform({ 0.5,-0.5,0 })));

		m_scene->AddLight(std::make_shared<Light>(glm::vec3(4, 4, 4), glm::vec3(1, 1, 1)));

		m_window->SetClearColor({ 0.05,0.05,0.05,1.0 });

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

		while (!m_window->IsCloseRequested()) {
			m_window->Clear();

			Input::Update(0.16f);
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
		glViewport(0, 0, e.GetWidth(), e.GetHeight());
	}


}