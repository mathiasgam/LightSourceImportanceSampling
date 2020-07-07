#pragma once

#include "Window.h"

#include "glm.hpp"
#include "CL/cl.h"

#include <iostream>

namespace LSIS {

	class Application {
	public:
		Application();
		virtual ~Application();

		void run();

	private:

	private:
		Window window;


	};

}