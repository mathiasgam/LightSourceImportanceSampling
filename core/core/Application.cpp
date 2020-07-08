#include "Application.h"

#include "glm.hpp"
#include "scene/Camera.h"

#include <iostream>

namespace LSIS {

	bool Initialized = false;

	Window m_window;
	std::unique_ptr<Scene> m_scene;
	std::shared_ptr<Camera> m_cam;

	void Application::Init()
	{
		m_window = Window("LSIS", { 720,512 });
		std::cout << "Created Application\n";

		m_scene = std::make_unique<Scene>();

		m_cam = std::make_shared<Camera>(glm::vec3(0, 0, 0.1), glm::vec3(0, 0, 0));
		m_scene->SetCamera(m_cam);

		auto flat = Shader::Create("kernels/flat.vert", "kernels/flat.frag");
		auto m1 = std::make_shared<Material>(flat, glm::vec4(1, 0, 0, 1));
		auto m2 = std::make_shared<Material>(flat, glm::vec4(0, 1, 0, 1));
		auto m3 = std::make_shared<Material>(flat, glm::vec4(0, 0, 1, 1));
		auto m4 = std::make_shared<Material>(flat, glm::vec4(1, 1, 1, 1));

		std::shared_ptr<Mesh> square = Mesh::CreateRect({ 0.5,0.5 });

		m_scene->AddObject(std::make_shared<Object>(square, m1, Transform({ -0.5,-0.5,0 })));
		m_scene->AddObject(std::make_shared<Object>(square, m2, Transform({ 0.5,0.5,0 })));
		m_scene->AddObject(std::make_shared<Object>(square, m3, Transform({ -0.5,0.5,0 })));
		m_scene->AddObject(std::make_shared<Object>(square, m4, Transform({ 0.5,-0.5,0 })));

		m_window.SetClearColor({ 0.05,0.05,0.05,1.0 });

		Initialized = true;
	}

	void Application::Run()
	{
		std::cout << "Running\n";

		std::cout << "Num Objects: " << m_scene->GetNumObjects() << std::endl;
		std::cout << "Num Lights:  " << m_scene->GetNumLights() << std::endl;

		//window.ReloadShaders();

		while (!m_window.IsCloseRequested()) {
			m_window.Clear();

			m_scene->Render();

			// Do rendering

			m_window.Update();
			m_window.SwapBuffers();
			m_window.WaitForEvent();
		}
	}

}