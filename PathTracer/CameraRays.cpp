#include "pch.h"
#include "CameraRays.h"

namespace LSIS {


	CameraRays::CameraRays(uint32_t width, uint32_t height, uint32_t samples)
		: m_width(width), m_height(height), m_multi_sample(samples)
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
	}

	void CameraRays::CompileKernels()
	{
	}

}