#pragma once

#include "Window.h"
#include "scene/Scene.h"

#include "glm.hpp"
#include "CL/cl.h"

#include <iostream>
#include <memory>

namespace LSIS {

	class Application {
	public:
		Application();
		virtual ~Application();

		void run();

	private:

	private:
		Window window;

		std::unique_ptr<Scene> m_scene;


	};

}