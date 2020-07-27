#include "pch.h"
#include "CameraRays.h"

namespace LSIS {


	CameraRays::CameraRays(uint32_t width, uint32_t height, uint32_t samples)
		: m_width(width), m_height(height), m_multi_sample(samples), m_seed(0)
	{
		CompileKernels();
	}

	CameraRays::~CameraRays()
	{
	}

	void CameraRays::SetResolution(uint32_t width, uint32_t height)
	{
		m_width = width;
		m_height = height;
	}

	void CameraRays::SetMultiSample(uint32_t samples)
	{
		m_multi_sample = samples;
	}

	void CameraRays::GenerateRays(const TypedBuffer<SHARED::Ray>& ray_buffer, const TypedBuffer<SHARED::Intersection>& intersection_buffer)
	{
		// Test that the buffers are the correct size
		//assert(ray_buffer.Count() == m_width * m_height);
		//assert(intersection_buffer.Count() == m_width * m_height);

		// Prepare the kernel arguments
		m_kernel.setArg(0, sizeof(cl_uint), &m_width);
		m_kernel.setArg(1, sizeof(cl_uint), &m_height);
		m_kernel.setArg(2, sizeof(cl_uint), &m_multi_sample);
		m_kernel.setArg(3, sizeof(cl_uint), &m_seed);
		m_kernel.setArg(4, ray_buffer.GetBuffer());
		m_kernel.setArg(5, intersection_buffer.GetBuffer());

		// submit kernel
		cl_int err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel, cl::NullRange, cl::NDRange(m_width * m_height));

		// check for errors
		if (err != CL_SUCCESS) {
			std::cout << "Error [CameraRays]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
	}

	void CameraRays::CompileKernels()
	{
		m_program = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "../Assets/Kernels/camera_rays.cl");
		m_kernel = Compute::CreateKernel(m_program, "generate_rays");
	}

}