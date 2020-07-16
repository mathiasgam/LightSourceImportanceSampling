#pragma once

#include <string>
#include <vector>

#include "CL/cl.h"

namespace LSIS::Compute {

	namespace Device {
		std::string GetName(cl_device_id id);
		std::string GetVendor(cl_device_id id);
		std::string GetVersion(cl_device_id id);
		std::string GetProfile(cl_device_id id);
		std::vector<std::string> GetExtensions(cl_device_id id);

		cl_device_id GetDevice(cl_platform_id platform_id, std::vector<std::string> prefered_devices);
	

		std::string GetInfoString(cl_device_id id, cl_device_info type);
	};

}