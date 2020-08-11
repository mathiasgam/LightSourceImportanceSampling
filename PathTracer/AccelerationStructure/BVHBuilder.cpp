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
		m_kernel_refit = Compute::CreateKernel(m_program, "refit_bounds");
	}

	const std::pair<TypedBuffer<SHARED::Node>, TypedBuffer<SHARED::AABB>> BVHBuilder::Build(const TypedBuffer<SHARED::Vertex>& vertices, const TypedBuffer<SHARED::Face>& faces)
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
		TypedBuffer<SHARED::AABB> bounds = TypedBuffer<SHARED::AABB>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<cl_float3> scene_bounds = TypedBuffer<cl_float3>(context, CL_MEM_READ_WRITE, 2);
		TypedBuffer<morton_key> morton_codes = TypedBuffer<morton_key>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<morton_key> codes_sorted = TypedBuffer<morton_key>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<cl_uint> indices = TypedBuffer<cl_uint>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<SHARED::Node> nodes = TypedBuffer<SHARED::Node>(context, CL_MEM_READ_WRITE, num_nodes);
		TypedBuffer<SHARED::AABB> bboxes = TypedBuffer<SHARED::AABB>(context, CL_MEM_READ_WRITE, num_nodes);

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
		m_kernel_prepare.setArg(5, bounds.GetBuffer());
		err = queue.enqueueNDRangeKernel(m_kernel_prepare, cl::NullRange, cl::NDRange(global_size));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder prep]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

		// reduce scene bounds
		m_kernel_scene_bounds.setArg(0, sizeof(cl_uint), &num_faces);
		m_kernel_scene_bounds.setArg(1, bounds.GetBuffer());
		m_kernel_scene_bounds.setArg(2, scene_bounds.GetBuffer());
		err = queue.enqueueNDRangeKernel(m_kernel_scene_bounds, cl::NullRange, cl::NDRange(32), cl::NDRange(32));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder scene bounds]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

		// calc_morton codes
		m_kernel_morton_code.setArg(0, sizeof(cl_uint), &num_faces);
		m_kernel_morton_code.setArg(1, centers.GetBuffer());
		m_kernel_morton_code.setArg(2, scene_bounds.GetBuffer());
		m_kernel_morton_code.setArg(3, morton_codes.GetBuffer());

		err = queue.enqueueNDRangeKernel(m_kernel_morton_code, cl::NullRange, cl::NDRange(global_size));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder morton]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
		
		// Sort codes
		auto source_buffer = morton_codes;
		auto target_buffer = codes_sorted;
		for (cl_uint shift = 0; shift < 64; shift += 8) {
			m_kernel_sort.setArg(0, sizeof(cl_uint), &num_faces);
			m_kernel_sort.setArg(1, sizeof(cl_uint), &shift);
			m_kernel_sort.setArg(2, source_buffer.GetBuffer());
			m_kernel_sort.setArg(3, target_buffer.GetBuffer());

			err = queue.enqueueNDRangeKernel(m_kernel_sort, cl::NullRange, cl::NDRange(32), cl::NDRange(32));
			auto tmp = source_buffer;
			source_buffer = target_buffer;
			target_buffer = tmp;
		}
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder sort]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

		codes_sorted = source_buffer;
		
		// Generate hierachy
		m_kernel_hireachy.setArg(0, sizeof(cl_uint), &num_faces);
		m_kernel_hireachy.setArg(1, sizeof(cl_uint), &num_nodes);
		m_kernel_hireachy.setArg(2, codes_sorted.GetBuffer());
		m_kernel_hireachy.setArg(3, bounds.GetBuffer());
		m_kernel_hireachy.setArg(4, nodes.GetBuffer());
		m_kernel_hireachy.setArg(5, bboxes.GetBuffer());

		err = queue.enqueueNDRangeKernel(m_kernel_hireachy, cl::NullRange, cl::NDRange(num_faces));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder hireachy]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

		// Refit bounding boxes
		cl::Buffer flags = cl::Buffer(context, CL_MEM_READ_WRITE, sizeof(cl_uint) * num_faces-1);
		err = queue.enqueueFillBuffer(flags, 0, 0, sizeof(cl_uint)* num_faces-1);
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder fill]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
		m_kernel_refit.setArg(0, sizeof(cl_uint), &num_faces);
		m_kernel_refit.setArg(1, nodes.GetBuffer());
		m_kernel_refit.setArg(2, bboxes.GetBuffer());
		m_kernel_refit.setArg(3, flags);

		err = queue.enqueueNDRangeKernel(m_kernel_refit, cl::NullRange, cl::NDRange(num_faces));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder refit]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

		// TODO copy nodes to readonly buffer
		/*
		auto out_nodes = TypedBuffer<SHARED::Node>(context, CL_MEM_READ_ONLY, num_nodes);
		auto out_bboxes = TypedBuffer<SHARED::AABB>(context, CL_MEM_READ_ONLY, num_nodes);
		queue.enqueueCopyBuffer(nodes.GetBuffer(), out_nodes.GetBuffer(), 0, 0, num_nodes);
		queue.enqueueCopyBuffer(bboxes.GetBuffer(), out_bboxes.GetBuffer(), 0, 0, num_nodes);
		*/
		queue.finish();
		return {nodes, bboxes};
	}
}