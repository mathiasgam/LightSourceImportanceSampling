#include "pch.h"
#include "Device.h"
#include "Platform.h"

namespace LSIS::Compute {

	std::string Device::GetName(cl_device_id id) 
	{
		return GetInfoString(id,CL_DEVICE_NAME);
	}

	std::string Device::GetVendor(cl_device_id id) 
	{
		return GetInfoString(id, CL_DEVICE_VENDOR);
	}

	std::string Device::GetVersion(cl_device_id id) 
	{
		return GetInfoString(id, CL_DEVICE_VERSION);
	}

	std::string Device::GetProfile(cl_device_id id)
	{
		return GetInfoString(id, CL_DEVICE_PROFILE);
	}

	std::vector<std::string> Device::GetExtensions(cl_device_id id) 
	{
		std::string string = GetInfoString(id, CL_DEVICE_EXTENSIONS);
		std::vector<std::string> result;

		std::string delim = " ";

		size_t start = 0U;
		size_t end = string.find(delim);

		while (end != std::string::npos) {
			result.push_back(string.substr(start, end - start));
			start = end + delim.length();
			end = string.find(delim, start);
		}

		return result;
	}

	cl_device_id Device::GetDevice(cl_platform_id platform_id, std::vector<std::string> prefered_devices)
	{
		if (platform_id == 0)
			return 0;
		for (auto& device_id : Platform::GetDevices(platform_id)) {
			std::string name = GetName(device_id);
			for (auto& prefered : prefered_devices) {
				if (name.compare(prefered))
					return device_id;
			}
		}
		return 0;
	}


	std::string Device::GetInfoString(cl_device_id id, cl_device_info type) 
	{
		size_t size;
		std::string string;

		// Get size of info string
		clGetDeviceInfo(id, type, 0, nullptr, &size);
		string.resize(size);

		// Load device info into string
		clGetDeviceInfo(id, type, size, string.data(), nullptr);

		return string;
	}

}