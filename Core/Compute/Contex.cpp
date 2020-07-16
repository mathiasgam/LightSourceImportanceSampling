#include "pch.h"
#include "Contex.h"

#include "Platform.h"

#include <iostream>
#include <vector>

namespace LSIS::Compute {


cl_context Context::CreateContext(const std::vector<cl_context_properties>& properties, cl_device_id device_id)
{
	cl_int err;
	cl_context context = clCreateContext(properties.data(), 1, &device_id, nullptr, nullptr, &err);

	// TODO handle errors

	return context;
}

}