#include "Application.h"

#include <iostream>

namespace LSIS {
	Application::Application()
	{
		std::cout << "Created Application\n";
	}

	Application::~Application()
	{
		std::cout << "Destroyed Application\n";
	}

	void Application::run()
	{
		std::cout << "Running\n";
	}

}