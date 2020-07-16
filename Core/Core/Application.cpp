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

#include "Compute/Compute.h"

#include <iostream>
#include <functional>

namespace LSIS {

	namespace Application {

		std::unique_ptr<Window> m_window;
		std::unique_ptr<Scene> m_scene;
		std::shared_ptr<Camera> m_cam;

		cl::Platform platform;
		cl::Device device;
		cl::Context context;
		cl::CommandQueue queue;

		std::vector<std::string> prefered_platforms = {
			"NVIDIA CUDA"
		};

		std::vector<std::string> prefered_devices = {
			"GeForce RTX 2080",
			"GeForce GTX 1050"
		};

		bool Initialized = false;

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
			// Find a prefered platform and device
			platform = Compute::GetPreferedPlatform(prefered_platforms);
			device = Compute::GetPreferedDevice(platform, prefered_devices);

			std::cout << "Platform: " << Compute::GetName(platform) << std::endl;
			std::cout << "Device: " << Compute::GetName(device) << std::endl;

			// Create Context
			context = Compute::CreateContext(m_window->GetCLProperties(platform()), device);
			queue = Compute::CreateCommandQueue(context, device);
		}

		void Init()
		{
			CreateWindow();
			CreateCLContext();

			LoadScene();

			std::cout << "Application Initialized\n";
			Initialized = true;
		}

		void Destroy()
		{
			std::cout << "Application Destroyed\n";
		}

		void UpdateCam() {
			m_cam->SetPosition(Input::GetCameraPosition());
			m_cam->SetRotation(Input::GetcameraRotation());
		}

		void Run()
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

		const cl::Context& GetContext()
		{
			return context;
		}

		const cl::Device& GetDevice()
		{
			return device;
		}

		const cl::CommandQueue& GetCommandQueue()
		{
			return queue;
		}

		void OnEvent(const Event& e)
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

		void OnWindowResizedEvent(const WindowResizeEvent& e)
		{
			RenderCommand::SetViewPort(0, 0, e.GetWidth(), e.GetHeight());
		}

	}

}