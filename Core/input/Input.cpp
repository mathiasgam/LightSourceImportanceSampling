#include "pch.h"
#include "Input.h"
#include "KeyCodes.h"

#include "Core/Application.h"

#include "gtc/matrix_transform.hpp"

namespace LSIS::Input {
	
	glm::vec4 cam_pos = glm::vec4(1, 1, 1, 1);
	glm::vec4 cam_vel = glm::vec4(0, 0, 0, 0);
	glm::vec3 cam_rot = glm::vec3(0, 0, 0);

	float cam_speed = 0.01f;
	float cam_speed_rot = 0.01f;

	bool OnKeyPressedEvent(const KeyPressedEvent& e){
		int key = e.GetKey();
		switch (key)
		{
		case KEY_A:
			cam_vel.x -= 1.0f;
			return true;
		case KEY_D:
			cam_vel.x += 1.0f;
			return true;
		case KEY_S:
			cam_vel.z += 1.0f;
			return true;
		case KEY_W:
			cam_vel.z -= 1.0f;
			return true;
		case KEY_SPACE:
			cam_vel.y += 1.0f;
			return true;
		case KEY_LEFT_CONTROL:
			cam_vel.y -= 1.0f;
			return true;
		case KEY_LEFT_SHIFT:
			cam_speed = 0.1f;
			return true;
		}
		return false;
	}

	bool OnKeyReleasedEvent(const KeyReleasedEvent& e) {
		int key = e.GetKey();
		switch (key)
		{
		case KEY_A:
			cam_vel.x += 1.0f;
			return true;
		case KEY_D:
			cam_vel.x -= 1.0f;
			return true;
		case KEY_S:
			cam_vel.z -= 1.0f;
			return true;
		case KEY_W:
			cam_vel.z += 1.0f;
			return true;
		case KEY_SPACE:
			cam_vel.y -= 1.0f;
			return true;
		case KEY_LEFT_CONTROL:
			cam_vel.y += 1.0f;
			return true;
		case KEY_LEFT_SHIFT:
			cam_speed = 0.01f;
			return true;
		}
		return false;
	}

	bool OnMouseMovedEvent(const MouseMovedEvent& e) {
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
		}

		lastX = x;
		lastY = y;
		return false;
	}

	glm::vec4 GetCameraPosition()
	{
		return cam_pos;
	}

	glm::vec4 GetCameraVelocity()
	{
		glm::mat4 R = glm::rotate(glm::rotate(glm::identity<glm::mat4>(), cam_rot.y, glm::vec3(0, 1, 0)), cam_rot.x, glm::vec3(1, 0, 0));
		return R * cam_vel * cam_speed;
	}

	glm::vec3 GetcameraRotation()
	{
		return cam_rot;
	}

	void SetCameraPosition(glm::vec4 position)
	{
		cam_pos = position;
	}

	void SetCameraRotation(glm::vec3 rotation)
	{
		cam_rot = rotation;
	}

	bool OnEvent(const Event& e)
	{
		EventType type = e.GetEventType();
		switch (type)
		{
		case EventType::KeyPressed:
			return OnKeyPressedEvent((const KeyPressedEvent&)e);
		case EventType::KeyReleased:
			return OnKeyReleasedEvent((const KeyReleasedEvent&)e);
		case EventType::MouseMoved:
			return OnMouseMovedEvent((const MouseMovedEvent&)e);
		}
		return false;
	}

	void Update(float delta)
	{
		cam_pos += GetCameraVelocity();
	}

}