#pragma once

#include "Window.h"
#include "scene/Scene.h"

#include "glm.hpp"
#include "CL/cl.h"

#include <iostream>
#include <memory>

#include "event/ApplicationEvent.h"
#include "event/MouseEvent.h"
#include "event/KeyEvent.h"

namespace LSIS {

	namespace Application {

		void Init();
		void Run();

		void OnEvent(const Event& e);

		void OnKeyPressedEvent(const KeyPressedEvent& e);
		void OnKeyReleasedEvent(const KeyReleasedEvent& e);
	}

}