#pragma once

#include "Window.h"
#include "Scene/Scene.h"

#include "glm.hpp"
#include "CL/cl.hpp"

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

		const cl::Context& GetContext();
		const cl::Device& GetDevice();
		const cl::CommandQueue& GetCommandQueue();

		void OnEvent(const Event& e);

		void OnWindowResizedEvent(const WindowResizeEvent& e);
	}

}