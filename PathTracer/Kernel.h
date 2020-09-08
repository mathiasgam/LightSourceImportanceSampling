#pragma once

#include "Compute/Compute.h"

#define KERNEL_PATH "Kernels/"

namespace LSIS {

	class Kernel {
	public:

		virtual void Compile() = 0;

	protected:

		// Utility function for making program loading easier
		virtual inline cl::Program LoadProgram(const std::string& name) final {
			std::string path = KERNEL_PATH;
			auto context = Compute::GetContext();
			auto device = Compute::GetDevice();
			return Compute::CreateProgram(context, device, path + name, { path });
		}
	};

}