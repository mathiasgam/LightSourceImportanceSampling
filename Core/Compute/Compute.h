#pragma once

#include "CL/cl.hpp"

#include "cl_error_code.h"

#ifdef DEBUG
#define CHECK(_openclcall_)															\
{																					\
	int err = _openclcall_;															\
	if (err != 0){																	\
		std::cout << "Error: " << err << ": " << GET_CL_ERROR_CODE(err) << ", line: " << __LINE__ << ", " << __FILE__ << "\n";		\
		exit(err);																	\
	}																				\
}
#else
#define CHECK(_openclcall_) _openclcall_
#endif // DEBUG

namespace LSIS {

	namespace Compute {

		// Platform
		std::string GetName(cl::Platform platform);
		std::string GetVendor(cl::Platform platform);
		std::string GetProfile(cl::Platform platform);
		std::string GetVersion(cl::Platform platform);
		std::vector<std::string> GetExtensions(cl::Platform platform);

		std::vector<cl::Platform> GetPlatforms();
		cl::Platform GetPreferedPlatform(std::vector<std::string> prefered_platforms);

		// Device
		std::string GetName(cl::Device device);
		std::string GetVendor(cl::Device device);
		std::string GetVersion(cl::Device device);
		std::string GetProfile(cl::Device device);
		std::vector<std::string> GetExtensions(cl::Device device);

		cl::Device GetPreferedDevice(cl::Platform platform, std::vector<std::string> prefered_devices);

		// Context
		cl::Context CreateContext(std::vector<cl_context_properties>& properties, const cl::Device& device);

		// Command queue
		cl::CommandQueue CreateCommandQueue(const cl::Context& context, const cl::Device& device);

		cl::Program CreateProgram(const cl::Context& context, const cl::Device& device, const std::string& filename);
		cl::Kernel CreateKernel(const cl::Program& program, const std::string& function_name);

	}
}