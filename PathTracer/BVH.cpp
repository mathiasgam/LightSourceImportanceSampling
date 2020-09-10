#include "pch.h"
#include "BVH.h"

namespace LSIS {


	BVH::BVH()
	{
		Compile();
	}

	BVH::~BVH()
	{
	}

	void BVH::SetGeometryBuffers(const TypedBuffer<SHARED::Vertex>& vertices, const TypedBuffer<SHARED::Face>& faces)
	{
		CHECK(m_closest.setArg(2, faces.GetBuffer()));
		CHECK(m_closest.setArg(3, vertices.GetBuffer()));

		CHECK(m_occlusion.setArg(2, faces.GetBuffer()));
		CHECK(m_occlusion.setArg(3, vertices.GetBuffer()));
	}

	void BVH::SetBVHBuffer(const TypedBuffer<SHARED::Node>& nodes, const TypedBuffer<SHARED::AABB>& bboxes)
	{
		CHECK(m_closest.setArg(0, nodes.GetBuffer()));
		CHECK(m_closest.setArg(1, bboxes.GetBuffer()));

		CHECK(m_occlusion.setArg(0, nodes.GetBuffer()));
		CHECK(m_occlusion.setArg(1, bboxes.GetBuffer()));
	}

	void BVH::Compile()
	{
		m_program = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/bvh.cl", { "Kernels/" });
		m_closest = Compute::CreateKernel(m_program, "intersect_bvh");
		m_occlusion = Compute::CreateKernel(m_program, "occluded");
	}

	void BVH::Trace(const TypedBuffer<SHARED::Ray>& rays, const TypedBuffer<SHARED::Intersection>& intersections, const TypedBuffer<SHARED::GeometricInfo>& info, const TypedBuffer<cl_uint>& count)
	{
		// Get ray count;
		cl_uint num_rays = static_cast<cl_uint>(rays.Count());

		// safty check ray and intersection match
		if (intersections.Count() != num_rays) {
			std::cout << "Error: ray and intersection buffer does not match in size\n";
			return;
		}

		// Bind dynamic kernel arguments
		CHECK(m_closest.setArg(4, rays.GetBuffer()));
		CHECK(m_closest.setArg(5, sizeof(cl_uint), &num_rays));
		CHECK(m_closest.setArg(6, intersections.GetBuffer()));
		CHECK(m_closest.setArg(7, info.GetBuffer()));

		// submit kernel
		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_closest, cl::NullRange, cl::NDRange(num_rays)));
	}

	void BVH::TraceOcclusion(const TypedBuffer<SHARED::Ray>& rays, const TypedBuffer<cl_int>& hits, const TypedBuffer<cl_uint>& count) {
		cl_uint num_rays = static_cast<cl_uint>(rays.Count());

		// Bind dynamic kernel arguments
		CHECK(m_occlusion.setArg(4, rays.GetBuffer()));
		CHECK(m_occlusion.setArg(5, sizeof(cl_uint), &num_rays));
		CHECK(m_occlusion.setArg(6, hits.GetBuffer()));

		// Submit kernel
		CHECK(Compute::GetCommandQueue().enqueueNDRangeKernel(m_occlusion, cl::NullRange, cl::NDRange(num_rays)));
	}

}