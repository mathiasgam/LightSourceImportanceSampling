#include "pch.h"

#include "Core/Application.h"
#include "Core/Log.h"
#include "Input/Input.h"

#include "PathTracer.h"

#include "entt.hpp"
#include "Scene/Components.h"

#include "gtc/constants.hpp"
#include "gtx/rotate_vector.hpp"

#ifdef LSIS_PLATFORM_WIN
int setenv(const char* name, const char* value, int overwrite)
{
	int errcode = 0;
	if (!overwrite) {
		size_t envsize = 0;
		errcode = getenv_s(&envsize, NULL, 0, name);
		if (errcode || envsize) return errcode;
	}
	return _putenv_s(name, value);
}
#endif // LSIS_PLATFORM_WIN

int main(int argc, char** argv) {

//#ifdef DEBUG
	setenv("CUDA_CACHE_DISABLE", "1", 1);
//#endif // DEBUG

	LSIS::Log::Init();

	LSIS::Application* app = LSIS::Application::Get();
	LSIS::Input::SetCameraPosition({ -0.0f,1.0f,2.72f,1.0f });
	LSIS::Input::SetCameraRotation({ -0.0f,-0.0f,0.0f });
	//LSIS::Input::SetCameraPosition({ 0.6f,0.8f,0.6f,1.0f });
	//LSIS::Input::SetCameraRotation({ -0.4f,0.5f,0.0f });
	//LSIS::Input::SetCameraPosition({ -0.0f,1.1f,1.3f,1.0f });
	//LSIS::Input::SetCameraRotation({ -0.5f,-0.0f,0.0f });

	std::shared_ptr<LSIS::PathTracer> pt = std::make_shared<LSIS::PathTracer>(512, 512);
	app->AddLayer(pt);

	// Run the program
	app->Run();
	app->Destroy();

	return 0;
}