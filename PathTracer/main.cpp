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

void save_profile(LSIS::PathTracer::profile_data profile, std::string filepath) {
	std::ofstream file;
	file.open(filepath, std::ios::out);

	if (file.is_open()) {
		//file << "host, " << profile.host << std::endl;
		file << "device, " << profile.device << std::endl;
		file << "platform, " << profile.platform << std::endl;
		file << "sampling, " << profile.sampling << std::endl;
		file << "attenuation, " << profile.attenuation << std::endl;
		file << "num_samples, " << profile.samples << std::endl;
		file << "time_build_lightstructure, " << profile.time_build_lightstructure << std::endl;
		file << "time_build_bvh, " << profile.time_build_bvh << std::endl;
		file << "time_render, " << profile.time_render << std::endl;

		file.close();
	}
	else {
		printf("Failed to create file!\n");
	}
}

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
	//LSIS::Input::SetCameraPosition({ -0.0f,1.0f,2.72f,1.0f });
	//LSIS::Input::SetCameraRotation({ -0.0f,-0.0f,0.0f });
	//LSIS::Input::SetCameraPosition({ 0.6f,0.8f,0.6f,1.0f });
	//LSIS::Input::SetCameraRotation({ -0.4f,0.5f,0.0f });
	LSIS::Input::SetCameraPosition({ -0.0f,1.1f,1.3f,1.0f });
	LSIS::Input::SetCameraRotation({ -0.5f,-0.0f,0.0f });

	auto flat = LSIS::Shader::Create("../Assets/Shaders/flat.vert", "../Assets/Shaders/flat.frag");
	auto m1 = std::make_shared<LSIS::Material>(flat, glm::vec4(199 / 256.0f, 151 / 256.0f, 40 / 256.0f, 1.0f));


	bool interactive = false;
	auto pt_method = LSIS::PathTracer::Method::lighttree;
	auto pt_attenuation = LSIS::PathTracer::ClusterAttenuation::ConditionalMinDist;

	std::string output_folder = "../Test/";
	std::string output_name = "Test";

	size_t sample_target = 100;
	auto scene = app->GetScene();

	for (int i = 1; i < argc; i++) {
		const std::string& arg = arg_list[i];
		//printf("Arg: %s\n", arg.c_str());
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
		else if (arg == "-method") {
			const std::string& method = arg_list[++i];
			if (method == "naive") {
				pt_method = LSIS::PathTracer::Method::naive;
			}
			else if (method == "energy") {
				pt_method = LSIS::PathTracer::Method::energy;
			}
			else if (method == "lighttree") {
				pt_method = LSIS::PathTracer::Method::lighttree;
			}
			else {
				printf("Unknown option\n");
			}
		}
		else if (arg == "-atten") {
			const std::string& atten = arg_list[++i];
			if (atten == "center") {
				pt_attenuation = LSIS::PathTracer::ClusterAttenuation::Center;
			}
			else if (atten == "conditional") {
				pt_attenuation = LSIS::PathTracer::ClusterAttenuation::Conditional;
			}
			else if (atten == "mindist") {
				pt_attenuation = LSIS::PathTracer::ClusterAttenuation::ConditionalMinDist;
			}
			else {
				printf("unknown cluster attenuation method\n");
			}
		}
		else if (arg == "-name") {
			const std::string& name = arg_list[++i];
			output_name = name;
		}
		else if (arg == "-outdir") {
			const std::string& out_folder = arg_list[++i];
			output_folder = out_folder;
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
		pt->SetMethod(pt_method);
		pt->SetClusterAttenuation(pt_attenuation);

		printf("Sleeping\n");
		std::cout << std::flush;

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

			// Print at every percent
			size_t print_interval = std::max<size_t>(10, sample_target / 100);
			size_t next_print = print_interval;

			pt->ResetSamples();
			size_t num_samples = 0;
			while (num_samples < sample_target) {
				pt->ProcessPass();
				//pt->UpdateRenderTexture();
				//printf("samples: %d\n", i++);

				num_samples = pt->GetNumSamples();
				if (num_samples >= next_print) {
					next_print += print_interval;
					float percent = ((float)num_samples / (float)sample_target) * 100.0f;
					printf("[%zd,%zd]: %.2f pct\n", num_samples, sample_target, percent);
					std::cout << std::flush;
				}
				if (num_samples >= sample_target) {
					LSIS::Compute::GetCommandQueue().finish();
				}
			}
			const auto end = std::chrono::high_resolution_clock::now();
			const std::chrono::duration<double, std::milli> duration = end - start;
			render_time = duration.count();

		}
		printf("Rendering Complete\n");

		const auto data = pt->GetPixelBufferData();
		save_result(data, output_folder + output_name + ".csv");

		auto profile = pt->GetProfileData();
		profile.time_render = render_time;

		save_profile(profile, output_folder + output_name + "_profile.csv");

		cl_ulong time_kernel_total = 0;
		time_kernel_total += profile.time_kernel_prepare;
		time_kernel_total += profile.time_kernel_shade;
		time_kernel_total += profile.time_kernel_trace;
		time_kernel_total += profile.time_kernel_process_occlusion;
		time_kernel_total += profile.time_kernel_process_results;
		double time_kernel_total_ms = (double)time_kernel_total / 1000000.0;
		double time_kernel_overhead = render_time - time_kernel_total_ms;

		printf("Results:\n");
		printf("- Render Time       : %fms\n", render_time);
		printf("- Render Overhead   : %fms\n", time_kernel_overhead);
		printf("  - kernel_prep     : %fms\n", (double)profile.time_kernel_prepare / 1000000.0);
		printf("  - kernel_trace    : %fms\n", (double)profile.time_kernel_trace / 1000000.0);
		printf("  - kernel_shade    : %fms\n", (double)profile.time_kernel_shade / 1000000.0);
		printf("  - kernel_p_occ    : %fms\n", (double)profile.time_kernel_process_occlusion / 1000000.0);
		printf("  - kernel_p_res    : %fms\n", (double)profile.time_kernel_process_results / 1000000.0);
		printf("- Build BVH         : %fms\n", profile.time_build_bvh);
		printf("- Build Lighttree   : %fms\n", profile.time_build_lightstructure);
		printf("- Num Samples       : %zd\n", profile.samples);
	}

	app->Destroy();

	return 0;
}