#pragma once

#include <vector>
#include <memory>
#include <string>

#include "CL/cl.h"

#include "Device.h"

namespace LSIS::Compute {

	namespace Platform {

		std::string GetName(cl_platform_id id);
		std::string GetVendor(cl_platform_id id);
		std::string GetProfile(cl_platform_id id);
		std::string GetVersion(cl_platform_id id);
		std::vector<std::string> GetExtensions(cl_platform_id id);

		std::vector<cl_device_id> GetDevices(cl_platform_id id);

		cl_platform_id GetPlatform(std::vector<std::string> name);
		std::vector<cl_platform_id> GetPlatformIDs();

		std::string GetInfoString(cl_platform_id id, cl_platform_info type);


	}
}