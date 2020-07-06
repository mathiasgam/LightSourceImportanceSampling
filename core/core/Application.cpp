#include "Application.h"

#include <iostream>

namespace LSIS {
	Application::Application()
		: window("App", { 720, 512 })
	{
		std::cout << "Created Application\n";

		window.SetClearColor({ 0.1,0.1,0.1,1.0 });
	}

	Application::~Application()
	{
		std::cout << "Destroyed Application\n";
	}

	void Application::run()
	{
		std::cout << "Running\n";
		while (!window.IsCloseRequested()) {
			window.Clear();

			// Do rendering

			window.Update();
			window.SwapBuffers();
		}
	}

}