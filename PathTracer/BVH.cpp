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
		cl_uint num_vertices = static_cast<cl_uint>(vertices.Count());
		cl_uint num_faces = static_cast<cl_uint>(faces.Count());

		m_kernel.setArg(2, faces.GetBuffer());
		m_kernel.setArg(3, vertices.GetBuffer());
		m_kernel.setArg(6, sizeof(uint32_t), &num_faces);
		m_kernel.setArg(7, sizeof(uint32_t), &num_vertices);
	}

	void BVH::SetBVHBuffer(const TypedBuffer<SHARED::Node>& nodes, const TypedBuffer<SHARED::AABB>& bboxes)
	{
		cl_uint num_nodes = static_cast<cl_uint>(nodes.Count());

		m_kernel.setArg(0, nodes.GetBuffer());
		m_kernel.setArg(1, bboxes.GetBuffer());
		m_kernel.setArg(5, sizeof(uint32_t), &m_num_nodes);
	}

	void BVH::Compile()
	{
		m_program = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/bvh.cl", { "Kernels/" });
		m_kernel = Compute::CreateKernel(m_program, "intersect_bvh");
	}

	void BVH::Trace(const TypedBuffer<SHARED::Ray>& rays, const TypedBuffer<SHARED::Intersection>& intersections)
	{
		// Get ray count;
		cl_uint num_rays = static_cast<cl_uint>(rays.Count());

		// safty check ray and intersection match
		if (intersections.Count() != num_rays) {
			std::cout << "Error: ray and intersection buffer does not match in size\n";
			return;
		}

		// Bind dynamic kernel arguments
		m_kernel.setArg(4, rays.GetBuffer());
		m_kernel.setArg(8, sizeof(uint32_t), &num_rays);
		m_kernel.setArg(9, intersections.GetBuffer());

		// submit kernel
		cl_int err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel, cl::NullRange, cl::NDRange(num_rays));

		// check for errors
		if (err != CL_SUCCESS) {
			std::cout << "Error: [BVH]" << GET_CL_ERROR_CODE(err) << std::endl;
		}
	}

}