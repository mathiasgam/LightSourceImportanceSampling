#include "pch.h"

#include "Core/Application.h"
#include "Input/Input.h"

#include "PathTracer.h"

#include "entt.hpp"
#include "Scene/Components.h"

int main(int argc, char** argv) {

	LSIS::Application* app = LSIS::Application::Get();
	LSIS::Input::SetCameraPosition({ -3.0f,1.0f,3.0f,1.0f });
	LSIS::Input::SetCameraRotation({ 0.0f,-0.6f,0.0f });

	std::shared_ptr<LSIS::PathTracer> pt = std::make_shared<LSIS::PathTracer>(720, 512);
	app->AddLayer(pt);

	app->Run();
	app->Destroy();

	return 0;
}