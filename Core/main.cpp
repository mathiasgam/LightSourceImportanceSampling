#include "pch.h"
#include <iostream>

#include "Core/Application.h"

int main(int argc, char** argv) {

	LSIS::Application::Init();
	LSIS::Application::Run();
	LSIS::Application::Destroy();

	return 0;
}