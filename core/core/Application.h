#pragma once

#include "Window.h"

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