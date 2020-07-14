#include "Application.h"

#include "Core.h"
#include "input/KeyCodes.h"
#include "input/Input.h"

#include "glm.hpp"
#include "gtc/matrix_transform.hpp"
#include "glad/glad.h"

#include "scene/Camera.h"

#include "mesh/MeshLoader.h"

#include <iostream>
#include <functional>

namespace LSIS {

	bool Initialized = false;

	std::unique_ptr<Window> m_window;
	std::unique_ptr<Scene> m_scene;
	std::shared_ptr<Camera> m_cam;

	glm::vec4 cam_pos = glm::vec4(1, 1, 1, 1);
	glm::vec4 cam_vel = glm::vec4(0, 0, 0, 0);
	glm::vec3 cam_rot = glm::vec3(0, 0, 0);

	float cam_speed = 0.01f;
	float cam_speed_rot = 0.01f;

	void Application::Init()
	{
		m_window = std::make_unique<Window>();
		m_window->SetEventCallback(Application::OnEvent);
		std::cout << "Created Application\n";

		m_scene = std::make_unique<Scene>();

		m_cam = std::make_shared<Camera>(cam_pos, cam_rot);
		m_scene->SetCamera(m_cam);

		auto flat = Shader::Create("kernels/flat.vert", "kernels/flat.frag");
		auto m1 = std::make_shared<Material>(flat, glm::vec4(1, 0, 0, 1));
		auto m2 = std::make_shared<Material>(flat, glm::vec4(0, 1, 0, 1));
		auto m3 = std::make_shared<Material>(flat, glm::vec4(0, 0, 1, 1));
		auto m4 = std::make_shared<Material>(flat, glm::vec4(1, 1, 1, 1));

		std::shared_ptr<MeshData> square = MeshLoader::CreateCube(0.5f);
		//auto bunny = MeshLoader::LoadFromOBJ("../models/bunny.obj");

		m_scene->LoadObject("../models/bunny.obj", m1, Transform({ -0.5,-0.5,0 }));
		m_scene->AddObject(std::make_shared<Object>(square, m2, Transform({ 0.5,0.5,0 })));
		m_scene->AddObject(std::make_shared<Object>(square, m3, Transform({ -0.5,0.5,0 })));
		m_scene->AddObject(std::make_shared<Object>(square, m4, Transform({ 0.5,-0.5,0 })));

		m_scene->AddLight(std::make_shared<Light>(glm::vec3(4,4,4), glm::vec3(1,1,1)));

		m_window->SetClearColor({ 0.05,0.05,0.05,1.0 });

		Initialized = true;
	}

	void UpdateCam() {
		glm::mat4 R = glm::rotate(glm::rotate(glm::identity<glm::mat4>(), cam_rot.y, glm::vec3(0, 1, 0)), cam_rot.x, glm::vec3(1, 0, 0));

		cam_pos += R * cam_vel * cam_speed;
		m_cam->SetPosition(cam_pos);
	}

	void Application::Run()
	{
		std::cout << "Running\n";

		std::cout << "Num Objects: " << m_scene->GetNumObjects() << std::endl;
		std::cout << "Num Lights:  " << m_scene->GetNumLights() << std::endl;

		//window.ReloadShaders();

		while (!m_window->IsCloseRequested()) {
			m_window->Clear();
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
		switch (type)
		{
		case EventType::KeyPressed:
			OnKeyPressedEvent((const KeyPressedEvent&)e);
			break;
		case EventType::KeyReleased:
			OnKeyReleasedEvent((const KeyReleasedEvent&)e);
			break;
		case EventType::MouseMoved:
			OnMouseMovedEvent((const MouseMovedEvent&)e);
			break;
		case EventType::WindowResize:
			OnWindowResizedEvent((const WindowResizeEvent&)e);
			break;
		default:
			std::cout << e << std::endl;
			break;
		}
	}

	void Application::OnKeyPressedEvent(const KeyPressedEvent& e)
	{
		int key = e.GetKey();
		switch (key)
		{
		case KEY_A:
			cam_vel.x -= 1.0f;
			break;
		case KEY_D:
			cam_vel.x += 1.0f;
			break;
		case KEY_S:
			cam_vel.z += 1.0f;
			break;
		case KEY_W:
			cam_vel.z -= 1.0f;
			break;
		case KEY_SPACE:
			cam_vel.y += 1.0f;
			break;
		case KEY_LEFT_CONTROL:
			cam_vel.y -= 1.0f;
			break;
		case KEY_LEFT_SHIFT:
			cam_speed = 0.1f;
			break;
		}

		//std::cout << "Key pressed: " << GetKeyString(key) << std::endl;

	}

	void Application::OnKeyReleasedEvent(const KeyReleasedEvent& e)
	{
		int key = e.GetKey();
		switch (key)
		{
		case KEY_A:
			cam_vel.x += 1.0f;
			break;
		case KEY_D:
			cam_vel.x -= 1.0f;
			break;
		case KEY_S:
			cam_vel.z -= 1.0f;
			break;
		case KEY_W:
			cam_vel.z += 1.0f;
			break;
		case KEY_SPACE:
			cam_vel.y -= 1.0f;
			break;
		case KEY_LEFT_CONTROL:
			cam_vel.y += 1.0f;
			break;
		case KEY_LEFT_SHIFT:
			cam_speed = 0.01f;
			break;
		}
		//std::cout << "Key released: " << GetKeyString(key) << std::endl;
	}

	void Application::OnMouseMovedEvent(const MouseMovedEvent& e)
	{
		static float lastX = e.GetX();
		static float lastY = e.GetY();

		const float x = e.GetX();
		const float y = e.GetY();
		const int mods = e.GetMods();

		if (mods & BIT(0)) {
			std::cout << "right drag\n";
		}
		else if (mods & BIT(1)) {
			const float diffX = x - lastX;
			const float diffY = y - lastY;
			cam_rot.y -= diffX * 0.01f;
			cam_rot.x = glm::clamp(cam_rot.x - diffY * 0.01f, -3.14f, 3.14f);

			m_cam->SetRotation(cam_rot);
		}

		lastX = x;
		lastY = y;
	}

	void Application::OnWindowResizedEvent(const WindowResizeEvent& e)
	{
		glViewport(0, 0, e.GetWidth(), e.GetHeight());
	}


}