#include <iostream>

#include "Core/Application.h"
#include "Core/PathTracer.h"

int main(int argc, char** argv) {

	LSIS::Application app = LSIS::Application();

	std::shared_ptr<LSIS::PathTracer> pt = std::make_shared<LSIS::PathTracer>(app.GetContext(), 512, 512);
	app.AddLayer(pt);

	//app.Init();
	app.Run();
	app.Destroy();

	return 0;
}