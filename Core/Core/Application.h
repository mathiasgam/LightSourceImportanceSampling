#pragma once

#include "Window.h"
#include "Scene/Scene.h"

#include "glm.hpp"
#include "CL/cl.h"

#include <iostream>
#include <memory>

#include "Event/ApplicationEvent.h"
#include "Event/MouseEvent.h"
#include "Event/KeyEvent.h"

namespace LSIS {

	namespace Application {

		void Init();
		void Destroy();
		void Run();

		void OnEvent(const Event& e);

		void OnWindowResizedEvent(const WindowResizeEvent& e);
	}

}