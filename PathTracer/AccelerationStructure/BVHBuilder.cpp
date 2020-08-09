#include "pch.h"
#include "BVHBuilder.h"

#include "Core/Timer.h"

namespace LSIS {

	BVHBuilder::BVHBuilder()
	{
		Compile();
	}

	BVHBuilder::~BVHBuilder()
	{
	}

	void BVHBuilder::Compile()
	{
		m_program = Compute::CreateProgram(Compute::GetContext(), Compute::GetDevice(), "Kernels/bvh_builder.cl", { "Kernels/" });
		m_kernel_prepare = Compute::CreateKernel(m_program, "prepare_geometry_data");
		m_kernel_scene_bounds = Compute::CreateKernel(m_program, "calc_scene_bounds");
		m_kernel_morton_code = Compute::CreateKernel(m_program, "generate_morton_codes");
		m_kernel_sort = Compute::CreateKernel(m_program, "sort_codes");
		m_kernel_hireachy = Compute::CreateKernel(m_program, "generate_hierachy");
	}

	const TypedBuffer<SHARED::Node> BVHBuilder::Build(const TypedBuffer<SHARED::Vertex>& vertices, const TypedBuffer<SHARED::Face>& faces)
	{
		PROFILE_SCOPE("BVH Build GPU");
		cl_int err;

		typedef struct morton_key {
			cl_ulong code;
			cl_uint index;
		} morton_key;

		cl_uint num_vertices = vertices.Count();
		cl_uint num_faces = faces.Count();
		size_t num_nodes = num_faces * 2 - 1;
		std::cout << "GPU BVH Build\n";

		auto& context = Compute::GetContext();
		auto& queue = Compute::GetCommandQueue();

		// Allocate temporary buffers
		TypedBuffer<cl_float3> centers = TypedBuffer<cl_float3>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<cl_float3> p_mins = TypedBuffer<cl_float3>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<cl_float3> p_maxs = TypedBuffer<cl_float3>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<cl_float3> bounds = TypedBuffer<cl_float3>(context, CL_MEM_READ_WRITE, 2);
		TypedBuffer<cl_ulong> morton_codes = TypedBuffer<cl_ulong>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<cl_ulong> codes_sorted = TypedBuffer<cl_ulong>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<cl_uint> indices = TypedBuffer<cl_uint>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<SHARED::Node> nodes = TypedBuffer<SHARED::Node>(context, CL_MEM_READ_WRITE, num_nodes);

		//size_t local_size = 32;
		//size_t num_groups = (num_faces / local_size) + (num_faces % local_size > 0);
		//size_t num_groups = 2;
		//size_t global_size = num_groups * local_size;
		size_t global_size = num_faces;

		// Prepare geometry data
		m_kernel_prepare.setArg(0, sizeof(cl_uint), &num_vertices);
		m_kernel_prepare.setArg(1, sizeof(cl_uint), &num_faces);
		m_kernel_prepare.setArg(2, vertices.GetBuffer());
		m_kernel_prepare.setArg(3, faces.GetBuffer());
		m_kernel_prepare.setArg(4, centers.GetBuffer());
		m_kernel_prepare.setArg(5, p_mins.GetBuffer());
		m_kernel_prepare.setArg(6, p_maxs.GetBuffer());
		err = queue.enqueueNDRangeKernel(m_kernel_prepare, cl::NullRange, cl::NDRange(global_size));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

		// reduce scene bounds
		m_kernel_scene_bounds.setArg(0, sizeof(cl_uint), &num_faces);
		m_kernel_scene_bounds.setArg(1, p_mins.GetBuffer());
		m_kernel_scene_bounds.setArg(2, p_maxs.GetBuffer());
		m_kernel_scene_bounds.setArg(3, bounds.GetBuffer());
		err = queue.enqueueNDRangeKernel(m_kernel_scene_bounds, cl::NullRange, cl::NDRange(32), cl::NDRange(32));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

		// calc_morton codes
		m_kernel_morton_code.setArg(0, sizeof(cl_uint), &num_faces);
		m_kernel_morton_code.setArg(1, centers.GetBuffer());
		m_kernel_morton_code.setArg(2, bounds.GetBuffer());
		m_kernel_morton_code.setArg(3, morton_codes.GetBuffer());

		err = queue.enqueueNDRangeKernel(m_kernel_morton_code, cl::NullRange, cl::NDRange(global_size));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
		

		cl_uint N = num_faces;
		std::vector<morton_key> A = std::vector<morton_key>(N);
		for (size_t i = 0; i < N; i++) {
			A[i].code = rand() % 0b11111111;
			A[i].index = i;
			//std::cout << "index: " << i << ", " << A[i] << std::endl;
		}

		auto temp_buffer_A = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(morton_key) * N);
		auto temp_buffer_B = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(morton_key) * N);
		queue.enqueueWriteBuffer(temp_buffer_A, CL_TRUE, 0, sizeof(morton_key) * N, (void*)A.data());

		auto source_buffer = temp_buffer_A;
		auto target_buffer = temp_buffer_B;
		for (cl_uint shift = 0; shift < 64; shift += 8) {

			// Sort codes
			m_kernel_sort.setArg(0, sizeof(cl_uint), &N);
			m_kernel_sort.setArg(1, sizeof(cl_uint), &shift);
			m_kernel_sort.setArg(2, source_buffer);
			m_kernel_sort.setArg(3, target_buffer);

			err = queue.enqueueNDRangeKernel(m_kernel_sort, cl::NullRange, cl::NDRange(1), cl::NDRange(1));
			auto tmp = source_buffer;
			source_buffer = target_buffer;
			target_buffer = tmp;
		}
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}


		//std::vector<morton_key> B = std::vector<morton_key>(N);
		//err = queue.enqueueReadBuffer(source_buffer, CL_TRUE, 0, sizeof(morton_key) * N, (void*)B.data());
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

		//std::sort(A.begin(), A.end(), [](morton_key a, morton_key b) {return a.code < b.code; });
		/*
		for (size_t i = 0; i < N; i++) {
			std::cout << "A: " << (uint32_t)A[i].code << ", B: " << (uint32_t)B[i].code << " index: " << (uint32_t)B[i].index << std::endl;
		}
		*/

		// Generate hierachy


		// TODO copy nodes to readonly buffer
		auto result = TypedBuffer<SHARED::Node>(context, CL_MEM_READ_ONLY, num_nodes);
		queue.enqueueCopyBuffer(nodes.GetBuffer(), result.GetBuffer(), 0, 0, num_nodes);

		queue.finish();
		return result;
	}
}