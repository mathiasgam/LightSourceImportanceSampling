#include "pch.h"
#include "Compute.h"

#include <iostream>

namespace LSIS::Compute {

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
	
	std::string GetName(cl::Platform platform)
	{
		return platform.getInfo<CL_PLATFORM_NAME>();
	}

	std::string GetVendor(cl::Platform platform)
	{
		return platform.getInfo<CL_PLATFORM_VENDOR>();
	}

	std::string GetProfile(cl::Platform platform)
	{
		return platform.getInfo<CL_PLATFORM_PROFILE>();
	}

	std::string GetVersion(cl::Platform platform)
	{
		return platform.getInfo<CL_PLATFORM_VERSION>();
	}

	std::vector<std::string> GetExtensions(cl::Platform platform)
	{
		std::string string = platform.getInfo<CL_PLATFORM_EXTENSIONS>();
		return SplitString(string, " ");
	}
	
	std::vector<cl::Platform> GetPlatforms()
	{
		std::vector<cl::Platform> platforms{};
		cl::Platform::get(&platforms);
		return platforms;
	}

	cl::Platform GetPreferedPlatform(std::vector<std::string> prefered_platforms)
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

	std::string GetName(cl::Device device)
	{
		return device.getInfo<CL_DEVICE_NAME>();
	}

	std::string GetVendor(cl::Device device)
	{
		return device.getInfo<CL_DEVICE_VENDOR>();
	}

	std::string GetVersion(cl::Device device)
	{
		return device.getInfo<CL_DEVICE_VERSION>();
	}

	std::string GetProfile(cl::Device device)
	{
		return device.getInfo<CL_DEVICE_PROFILE>();
	}

	std::vector<std::string> GetExtensions(cl::Device device)
	{
		std::string string = device.getInfo<CL_DEVICE_EXTENSIONS>();
		return SplitString(string, " ");
	}

	cl::Device GetPreferedDevice(cl::Platform platform, std::vector<std::string> prefered_devices)
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

	cl::Context CreateContext(std::vector<cl_context_properties>& properties, const cl::Device& device)
	{
		cl_int err;
		cl::Context context = cl::Context(device, properties.data(), nullptr, nullptr, &err);

		// TODO handle errors

		return context;
	}

	cl::CommandQueue CreateCommandQueue(const cl::Context& context, const cl::Device& device)
	{
		cl_int err;
		cl_command_queue_properties props{};
		cl::CommandQueue queue = cl::CommandQueue(context, device, props, &err);
		// TODO handle errors

		return queue;
	}

}