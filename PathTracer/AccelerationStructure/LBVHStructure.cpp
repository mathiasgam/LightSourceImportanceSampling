#include "pch.h"
#include "LBVHStructure.h"

namespace LSIS {

	LBVHStructure::LBVHStructure()
	{
		CompileKernels();
	}

	LBVHStructure::~LBVHStructure()
	{
	}

	void LBVHStructure::Build(const VertexData* vertices, size_t num_vertices, const uint32_t* indices, size_t num_indices)
	{
		std::cout << "Building LBVH\n";
		std::cout << "Num Vertices: " << num_vertices << std::endl;
		std::cout << "Num Indices: " << num_indices << std::endl;
	}

	void LBVHStructure::TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer)
	{
		// Get ray count;
		cl_uint num_rays = ray_buffer.Count();

		// safty check ray and intersection match
		if (intersection_buffer.Count() != num_rays) {
			std::cout << "Error: ray and intersection buffer does not match in size\n";
			return;
		}

		// Bind dynamic kernel arguments
		m_kernel.setArg(3, ray_buffer.GetBuffer());
		m_kernel.setArg(7, sizeof(uint32_t), &num_rays);
		m_kernel.setArg(8, intersection_buffer.GetBuffer());

		// submit kernel
		cl_int err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel, cl::NullRange, cl::NDRange(num_rays));

		// check for errors
		if (err != CL_SUCCESS) {
			std::cout << "Error: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
	}

	void LBVHStructure::CompileKernels()
	{
		m_program = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "../Assets/Kernels/bvh.cl");
		m_kernel = Compute::CreateKernel(m_program, "intersect_bvh");
	}

	void LBVHStructure::LoadGeometryBuffers(const Vertex* vertices, size_t num_vertices, const Face* faces, size_t num_faces)
	{
		// initialize buffers
		m_buffer_vertices = cl::Buffer(Compute::GetContext(), CL_MEM_READ_ONLY, sizeof(Vertex) * num_vertices);
		m_buffer_faces = cl::Buffer(Compute::GetContext(), CL_MEM_READ_ONLY, sizeof(Face) * num_faces);
		m_num_vertices = num_vertices;
		m_num_faces = num_faces;

		// set static kernel arguments
		m_kernel.setArg(1, m_buffer_faces);
		m_kernel.setArg(2, m_buffer_vertices);
		m_kernel.setArg(5, sizeof(uint32_t), &m_num_faces);
		m_kernel.setArg(6, sizeof(uint32_t), &m_num_vertices);
	}

	void LBVHStructure::LoadBVHBuffer(const Node* nodes, size_t num_nodes)
	{
		// initialize buffers
		m_buffer_bvh = cl::Buffer(Compute::GetContext(), CL_MEM_READ_ONLY, sizeof(Node) * num_nodes);
		m_num_nodes = num_nodes;

		// set static kernel arguments
		m_kernel.setArg(0, m_buffer_bvh);
		m_kernel.setArg(4, sizeof(uint32_t), &m_num_nodes);
	}

}