#include "Application.h"

#include "glm.hpp"
#include "scene/Camera.h"

#include <iostream>

namespace LSIS {

	Application::Application()
		: window("LSIS", { 720, 512 })
	{
		std::cout << "Created Application\n";

		m_scene = std::make_unique<Scene>();
		m_scene->SetCamera(std::make_shared<Camera>(glm::vec3(3, 3, 3), glm::vec3(1, 0, 0)));
		m_scene->AddMesh(Mesh::CreateSquare({ -0.5,-0.5,0 }, { 0.5,0.5 }, { 1,0,0 }));
		m_scene->AddMesh(Mesh::CreateSquare({ 0.5,0.5,0 }, { 0.5,0.5 }, { 0,1,0 }));
		m_scene->AddMesh(Mesh::CreateSquare({ 0.5, -0.5,0 }, { 0.5,0.5 }, { 0,0,1 }));
		m_scene->AddMesh(Mesh::CreateSquare({ -0.5, 0.5,0 }, { 0.5,0.5 }));

		window.SetClearColor({ 0.05,0.05,0.05,1.0 });

		m_scene->Upload();
	}

	Application::~Application()
	{
		std::cout << "Destroyed Application\n";
	}

	void Application::run()
	{
		std::cout << "Running\n";

		std::cout << "Num Meshes: " << m_scene->GetNumMeshes() << std::endl;
		std::cout << "Num Lights: " << m_scene->GetNumLights() << std::endl;

		//window.ReloadShaders();

		while (!window.IsCloseRequested()) {
			window.Clear();

			m_scene->Render();

			// Do rendering

			window.Update();
			window.SwapBuffers();
			window.WaitForEvent();
		}
	}



}