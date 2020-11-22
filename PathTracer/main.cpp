#include "pch.h"

#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/Timer.h"
#include "Input/Input.h"

#include "PathTracer.h"
#include "PathtracingLayer.h"

#include "entt.hpp"
#include "Scene/Components.h"

#include "gtc/constants.hpp"
#include "gtx/rotate_vector.hpp"

#include <chrono>
#include <thread>

#include <iostream>
#include <fstream>
#include <time.h>

#include "IO/Image.h"


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


void save_result(const std::vector<float>& data, std::string filepath) {
	if (data.size() % 4 != 0) {
		printf("Result format not recognized!\n");
	}

	std::ofstream file;
	file.open(filepath, std::ios::out);

	if (file.is_open()) {
		size_t num_pixels = data.size() / 4;
		for (size_t i = 0; i < num_pixels; i++) {
			file << data[i * 4 + 0] << ", " << data[i * 4 + 1] << ", " << data[i * 4 + 2] << "\n";
		}
		file.close();
	}
	else {
		printf("Failed to create file!\n");
	}

	int channels = 4;
	int width = 512;
	int height = 512;

	LSIS::SaveImageFromFloatBuffer("../Result.png", width, height, channels, data.data());
}

int main(int argc, char** argv) {

	std::vector<std::string> arg_list(argv, argc + argv);

	srand(time(NULL));

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

	auto flat = LSIS::Shader::Create("../Assets/Shaders/flat.vert", "../Assets/Shaders/flat.frag");
	auto m1 = std::make_shared<LSIS::Material>(flat, glm::vec4(199 / 256.0f, 151 / 256.0f, 40 / 256.0f, 1.0f));


	bool interactive = false;
	size_t sample_target = 10;
	auto scene = app->GetScene();

	for (int i = 1; i < argc; i++) {
		const std::string& arg = arg_list[i];
		printf("Arg: %s\n", arg.c_str());
		if (arg == "-obj") {
			const std::string& filepath = arg_list[++i];
			printf("Loading Object: %s\n", filepath.c_str());
			scene->LoadObject(filepath, m1, LSIS::Transform({ 0,0,0 }));
		}
		else if (arg == "-n") {
			const std::string& number = arg_list[++i];
			int n = std::stoi(number);
			printf("Set Target Number of samples: %s, %d\n", number, n);

			sample_target = n;
		}
		else {
			printf("Unknown argument: %s\n", arg);
		}
	}



	// Run the program
	if (interactive) {
		std::shared_ptr<LSIS::PathtracingLayer> pt = std::make_shared<LSIS::PathtracingLayer>(512, 512);
		app->AddLayer(pt);
		app->Run();
	}
	else {
		auto pt = std::make_shared<LSIS::PathTracer>(512, 512);
		
		printf("Sleeping\n");
		std::this_thread::sleep_for(std::chrono::milliseconds(10000));
		printf("Woke Up\n");

		LSIS::Input::Update(0.0f);
		scene->Update();
		app->UpdateCam();

		auto cam = scene->GetCamera();
		pt->SetCameraProjection(glm::transpose(glm::inverse(cam->GetViewProjectionMatrix())));

		LSIS::PROFILE_SCOPE("Total Time");

		pt->Reset();

		double render_time;

		printf("Start Rendering\n");
		{
			int i = 0;
			//pt->Reset();
			const auto start = std::chrono::high_resolution_clock::now();
			//LSIS::PROFILE_SCOPE("Render Time");
			pt->ResetSamples();
			while (pt->GetNumSamples() < sample_target) {
				pt->ProcessPass();
				//pt->UpdateRenderTexture();
				//printf("samples: %d\n", i++);
			}
			const auto end = std::chrono::high_resolution_clock::now();
			const std::chrono::duration<double, std::milli> duration = end - start;
			render_time = duration.count();
		}

		const auto data = pt->GetPixelBufferData();
		save_result(data, "../Test/Test.csv");

		const auto profile = pt->GetProfileData();
		printf("Render Time     : %fms\n", render_time);
		printf("Build BVH       : %fms\n", profile.time_build_bvh);
		printf("Build Lighttree : %fms\n", profile.time_build_lightstructure);
		printf("Num Samples     : %zd\n", profile.samples);
	}

	app->Destroy();

	return 0;
}