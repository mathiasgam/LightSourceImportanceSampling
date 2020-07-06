#pragma once

#include "Window.h"

#include "CL/cl.h"

namespace LSIS {

	class Application {
	public:
		Application();
		virtual ~Application();

		void run();

	private:
		Window window;


	};

}