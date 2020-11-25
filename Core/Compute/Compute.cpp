#include "pch.h"
#include "Compute.h"

#include <iostream>
#include <fstream>

namespace LSIS {

	struct ComputeData {
		cl::Platform platform;
		cl::Device device;
		cl::Context context;
		cl::CommandQueue queue;
	};

	static ComputeData s_Data;

	std::vector<std::string> SplitString(std::string string, std::string delim) {
		std::vector<std::string> result;

		size_t start = 0;
		size_t end = string.find(delim);

		while (end != std::string::npos) {
			result.push_back(string.substr(start, end - start));
			start = end + delim.length();
			end = string.find(delim, start);
		}
		return result;
	}
	
	std::string Compute::GetName(cl::Platform platform)
	{
		return platform.getInfo<CL_PLATFORM_NAME>();
	}

	std::string Compute::GetVendor(cl::Platform platform)
	{
		return platform.getInfo<CL_PLATFORM_VENDOR>();
	}

	std::string Compute::GetProfile(cl::Platform platform)
	{
		return platform.getInfo<CL_PLATFORM_PROFILE>();
	}

	std::string Compute::GetVersion(cl::Platform platform)
	{
		return platform.getInfo<CL_PLATFORM_VERSION>();
	}

	std::vector<std::string> Compute::GetExtensions(cl::Platform platform)
	{
		std::string string = platform.getInfo<CL_PLATFORM_EXTENSIONS>();
		return SplitString(string, " ");
	}
	
	std::vector<cl::Platform> Compute::GetPlatforms()
	{
		std::vector<cl::Platform> platforms{};
		cl::Platform::get(&platforms);
		return platforms;
	}

	cl::Platform Compute::GetPreferedPlatform(std::vector<std::string> prefered_platforms)
	{
		for (auto& platform_id : GetPlatforms()) {
			std::string name = GetName(platform_id);
			for (auto& prefered : prefered_platforms) {
				if (name.compare(prefered))
					return platform_id;
			}
		}
		std::cout << "Failed to find prefered platform\n";
		return cl::Platform::getDefault();
	}

	std::string Compute::GetName(cl::Device device)
	{
		return device.getInfo<CL_DEVICE_NAME>();
	}

	std::string Compute::GetVendor(cl::Device device)
	{
		return device.getInfo<CL_DEVICE_VENDOR>();
	}

	std::string Compute::GetVersion(cl::Device device)
	{
		return device.getInfo<CL_DEVICE_VERSION>();
	}

	std::string Compute::GetProfile(cl::Device device)
	{
		return device.getInfo<CL_DEVICE_PROFILE>();
	}

	std::vector<std::string> Compute::GetExtensions(cl::Device device)
	{
		std::string string = device.getInfo<CL_DEVICE_EXTENSIONS>();
		return SplitString(string, " ");
	}

	cl::Device Compute::GetPreferedDevice(cl::Platform platform, std::vector<std::string> prefered_devices)
	{
		std::vector<cl::Device> devices{};
		platform.getDevices(CL_DEVICE_TYPE_GPU, &devices);
		for (auto& device : devices) {
			std::string name = GetName(device);
			for (auto& prefered : prefered_devices) {
				if (name.compare(prefered))
					return device;
			}
		}
		std::cout << "Failed to find prefered device\n";
		return devices[0];
	}

	cl::Context Compute::CreateContext(std::vector<cl_context_properties>& properties, const cl::Device& device)
	{
		cl_int err;
		cl::Context context = cl::Context(device, properties.data(), nullptr, nullptr, &err);

		if (err) {
			std::cout << "ERROR: " << GET_CL_ERROR_CODE(err) << std::endl;
			exit(err);
		}

		return context;
	}

	cl::CommandQueue Compute::CreateCommandQueue(const cl::Context& context, const cl::Device& device)
	{
		cl_int err;
		cl_command_queue_properties props = CL_QUEUE_PROFILING_ENABLE;
		cl::CommandQueue queue = cl::CommandQueue(context, device, props, &err);

		if (err) {
			std::cout << "ERROR: " << GET_CL_ERROR_CODE(err) << std::endl;
			exit(err);
		}

		return queue;
	}

	std::string ReadFile(const std::string& filename) {
		auto filestream = std::ifstream(filename);
		if (filestream.fail()) {
			std::cerr << "Failed to open file: \"" << filename << "\n";
		}
		auto ss = std::stringstream();
		if (filestream.is_open()) {
			std::string line;
			while (std::getline(filestream, line)) {
				ss << line << "\n";
			}
		}
		ss << "\n#define CACHE_WORKAROUND " << std::rand() << "\n";
		return ss.str();
	}

	cl::Program Compute::CreateProgram(const cl::Context& context, const cl::Device& device, const std::string& filename, const std::vector<std::string>& options)
	{
		std::string str = ReadFile(filename);
		cl::Program::Sources source(1, std::make_pair(str.c_str(), str.length() + 1));
		cl::Program program(context, source);

		std::stringstream options_string{};
		for (auto option : options) {
			options_string << option << " ";
		}

#ifdef DEBUG
		options_string << "-D DEBUG";
#endif // DEBUG

		auto err = program.build(options_string.str().c_str());

		if (err) {
			std::cout << "Failed to build kernel program!!!\n";
			std::string buildlog = program.getBuildInfo<CL_PROGRAM_BUILD_LOG>(device);
			std::cerr << "Build log: " << GET_CL_ERROR_CODE(err) << "\n" << "at: " << filename << "\n" << buildlog << "\n";
		}
		return program;
	}

	cl::Kernel Compute::CreateKernel(const cl::Program& program, const std::string& function_name)
	{
		cl_int err;
		cl::Kernel kernel(program, function_name.c_str(), &err);
		if (err != 0) {
			std::cout << "Error: " << err << ": " << GET_CL_ERROR_CODE(err) << ", function: " << function_name << "\n";
		}
		return kernel;
	}

	const cl::Platform& Compute::GetPlatform()
	{
		return s_Data.platform;
	}

	const cl::Device& Compute::GetDevice()
	{
		return s_Data.device;
	}

	const cl::Context& Compute::GetContext()
	{
		return s_Data.context;
	}

	const cl::CommandQueue& Compute::GetCommandQueue()
	{
		return s_Data.queue;
	}

	cl::Platform& Compute::GetDynamicPlatform()
	{
		return s_Data.platform;
	}

	cl::Device& Compute::GetDynamicDevice()
	{
		return s_Data.device;
	}

	cl::Context& Compute::GetDynamicContext()
	{
		return s_Data.context;
	}

	cl::CommandQueue& Compute::GetDynamicCommandQueue()
	{
		return s_Data.queue;
	}

	void Compute::SetPlatform(cl::Platform& platform)
	{
		s_Data.platform = platform;
	}

	void Compute::SetDevice(cl::Device& device)
	{
		s_Data.device = device;
	}

	void Compute::SetContext(cl::Context& context)
	{
		s_Data.context = context;
	}

	void Compute::SetCommandQueue(cl::CommandQueue& queue)
	{
		s_Data.queue = queue;
	}

}