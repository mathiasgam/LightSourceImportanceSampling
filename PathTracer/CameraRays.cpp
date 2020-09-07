#include "pch.h"
#include "CameraRays.h"

#include "gtc/type_ptr.hpp"

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

	void CameraRays::GenerateRays(const TypedBuffer<SHARED::Ray>& ray_buffer, const TypedBuffer<SHARED::Sample>& sample_buffer)
	{
		m_seed++;
		// Test that the buffers are the correct size
		//assert(ray_buffer.Count() == m_width * m_height);
		//assert(intersection_buffer.Count() == m_width * m_height);

		auto cam = Application::Get()->GetScene()->GetCamera();
		auto cam_pos = cam->GetPosition();
		auto cam_rot = cam->GetRotation();

		//glm::mat4 cam_matrix = glm::transpose(cam->GetModelMatrix());
		glm::mat4 cam_matrix = glm::transpose(glm::inverse(cam->GetViewProjectionMatrix()));

		// Prepare the kernel arguments
		CHECK(m_kernel.setArg(0, sizeof(cl_uint), &m_width));
		CHECK(m_kernel.setArg(1, sizeof(cl_uint), &m_height));
		CHECK(m_kernel.setArg(2, sizeof(cl_uint), &m_multi_sample));
		CHECK(m_kernel.setArg(3, sizeof(cl_uint), &m_seed));
		CHECK(m_kernel.setArg(4, sizeof(cl_float4) * 4, glm::value_ptr(cam_matrix)));
		CHECK(m_kernel.setArg(5, ray_buffer.GetBuffer()));
		CHECK(m_kernel.setArg(6, sample_buffer.GetBuffer()));

		// submit kernel
		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel, cl::NullRange, cl::NDRange(m_width * m_height)));
	}

	void CameraRays::CompileKernels()
	{
		m_program = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/camera_rays.cl", { "Kernels/" });
		m_kernel = Compute::CreateKernel(m_program, "generate_rays");
	}

	size_t CameraRays::CalculateMemory() const
	{
		return 0;
	}

}