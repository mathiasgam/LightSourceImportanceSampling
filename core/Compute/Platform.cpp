#include "pch.h"
#include "Platform.h"

namespace LSIS::Compute {

	std::string Platform::GetInfoString(cl_platform_id id, cl_platform_info type)
	{
		size_t size;
		std::string info;

		// Get info size
		clGetPlatformInfo(id, type, 0, nullptr, &size);
		info.resize(size);

		// Load info into string
		clGetPlatformInfo(id, type, size, info.data(), nullptr);

		return info;
	}

	std::string Platform::GetName(cl_platform_id id)
	{
		return GetInfoString(id, CL_PLATFORM_NAME);
	}

	std::string Platform::GetVendor(cl_platform_id id)
	{
		return GetInfoString(id, CL_PLATFORM_VENDOR);
	}

	std::string Platform::GetProfile(cl_platform_id id)
	{
		return GetInfoString(id, CL_PLATFORM_PROFILE);
	}

	std::string Platform::GetVersion(cl_platform_id id)
	{
		return GetInfoString(id, CL_PLATFORM_VERSION);
	}

	std::vector<std::string> Platform::GetExtensions(cl_platform_id id)
	{
		std::string string = GetInfoString(id, CL_PLATFORM_EXTENSIONS);
		std::vector<std::string> result;
		
		std::string delim = " ";

		size_t start = 0;
		size_t end = string.find(delim);

		while (end != std::string::npos) {
			result.push_back(string.substr(start, end - start));
			start = end + delim.length();
			end = string.find(delim, start);
		}

		return result;
	}

	std::vector<cl_device_id> Platform::GetDevices(cl_platform_id id)
	{
		cl_uint num_devices;
		std::vector<cl_device_id> ids{};

		// Get number of devices
		clGetDeviceIDs(id, CL_DEVICE_TYPE_GPU, 0, nullptr, &num_devices);
		ids.resize(num_devices);

		// Load IDs into vector
		clGetDeviceIDs(id, CL_DEVICE_TYPE_GPU, num_devices, ids.data(), nullptr);

		return ids;
	}

	cl_platform_id Platform::GetPlatform(std::vector<std::string> prefered_platforms)
	{
		for (auto& platform_id : GetPlatformIDs()) {
			std::string name = GetName(platform_id);
			for (auto& prefered : prefered_platforms) {
				if (name.compare(prefered))
					return platform_id;
			}
		}
		return 0;
	}

	std::vector<cl_platform_id> Platform::GetPlatformIDs()
	{
		uint32_t num_platforms;
		std::vector<cl_platform_id> platform_ids{};

		// Get the number of platforms
		clGetPlatformIDs(0, nullptr, &num_platforms);
		platform_ids.resize(num_platforms);

		// Load the platform ids into the vector
		clGetPlatformIDs(num_platforms, platform_ids.data(), nullptr);

		return platform_ids;
	}


}
