#include "pch.h"
#include <iostream>

#include "Core/Application.h"

int main(int argc, char** argv) {

	LSIS::Application app = LSIS::Application();

	//app.Init();
	app.Run();
	app.Destroy();

	return 0;
}