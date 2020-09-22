#pragma once

#include "CL/cl.hpp"

#include "cl_error_code.h"

#ifdef DEBUG
#define CHECK(_openclcall_)															\
{																					\
	int err = _openclcall_;															\
	if (err != 0){																	\
		std::cout << "Error: " << err << ": " << GET_CL_ERROR_CODE(err) << ", line: " << __LINE__ << ", " << __FILE__ << "\n";		\
		__debugbreak();																\
	}																				\
}
#else
#define CHECK(_openclcall_) _openclcall_
#endif // DEBUG

#include "Core/Application.h"

namespace LSIS {

	class Compute {
		// Give application access to private methods
		friend class LSIS::Application;
	public:

		// Platform
		static std::string GetName(cl::Platform platform);
		static std::string GetVendor(cl::Platform platform);
		static std::string GetProfile(cl::Platform platform);
		static std::string GetVersion(cl::Platform platform);
		static std::vector<std::string> GetExtensions(cl::Platform platform);

		static std::vector<cl::Platform> GetPlatforms();
		static cl::Platform GetPreferedPlatform(std::vector<std::string> prefered_platforms);

		// Device
		static std::string GetName(cl::Device device);
		static std::string GetVendor(cl::Device device);
		static std::string GetVersion(cl::Device device);
		static std::string GetProfile(cl::Device device);
		static std::vector<std::string> GetExtensions(cl::Device device);

		static cl::Device GetPreferedDevice(cl::Platform platform, std::vector<std::string> prefered_devices);

		// Context
		static cl::Context CreateContext(std::vector<cl_context_properties>& properties, const cl::Device& device);

		// Command queue
		static cl::CommandQueue CreateCommandQueue(const cl::Context& context, const cl::Device& device);

		static cl::Program CreateProgram(const cl::Context& context, const cl::Device& device, const std::string& filename, const std::vector<std::string>& include_paths);
		static cl::Kernel CreateKernel(const cl::Program& program, const std::string& function_name);

		// Getters for static compute context
		static const cl::Platform& GetPlatform();
		static const cl::Device& GetDevice();
		static const cl::Context& GetContext();
		static const cl::CommandQueue& GetCommandQueue();

	private:
		static cl::Platform& GetDynamicPlatform();
		static cl::Device& GetDynamicDevice();
		static cl::Context& GetDynamicContext();
		static cl::CommandQueue& GetDynamicCommandQueue();

		static void SetPlatform(cl::Platform& platform);
		static void SetDevice(cl::Device& device);
		static void SetContext(cl::Context& context);
		static void SetCommandQueue(cl::CommandQueue& queue);

	};
}