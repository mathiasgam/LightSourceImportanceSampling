#include "pch.h"
#include "BVHBuilder.h"

#include "Core/Timer.h"

#include "DataStructures/MortonCode.h"

namespace LSIS {

	cl_float4 min(cl_float4 a, cl_float4 b) {
		return { std::min(a.x,b.x), std::min(a.y,b.y) ,std::min(a.z,b.z) ,std::min(a.w,b.w) };
	}

	cl_float4 max(cl_float4 a, cl_float4 b) {
		return { std::max(a.x,b.x), std::max(a.y,b.y) ,std::max(a.z,b.z) ,std::max(a.w,b.w) };
	}

	SHARED::AABB bbox_union(SHARED::AABB a, SHARED::AABB b) {
		SHARED::AABB bbox = {};
		bbox.min = min(a.min, b.min);
		bbox.max = max(a.max, b.max);
		return bbox;
	}

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

		cl_uint num_vertices = vertices.Count();
		cl_uint num_faces = faces.Count();
		size_t num_nodes = num_faces * 2 - 1;
		std::cout << "GPU BVH Build\n";

		auto& context = Compute::GetContext();
		auto& queue = Compute::GetCommandQueue();

		// Allocate temporary buffers
		TypedBuffer<cl_float3> centers = TypedBuffer<cl_float3>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<SHARED::AABB> prim_bounds = TypedBuffer<SHARED::AABB>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<cl_float3> scene_bounds = TypedBuffer<cl_float3>(context, CL_MEM_READ_WRITE, 2);
		TypedBuffer<morton_key> morton_codes = TypedBuffer<morton_key>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<morton_key> codes_sorted = TypedBuffer<morton_key>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<cl_uint> indices = TypedBuffer<cl_uint>(context, CL_MEM_READ_WRITE, num_faces);
		TypedBuffer<SHARED::Node> nodes = TypedBuffer<SHARED::Node>(context, CL_MEM_READ_WRITE, num_nodes);
		TypedBuffer<SHARED::AABB> nodes_bounds = TypedBuffer<SHARED::AABB>(context, CL_MEM_READ_WRITE, num_nodes);

		//size_t local_size = 32;
		//size_t num_groups = (num_faces / local_size) + (num_faces % local_size > 0);
		//size_t num_groups = 2;
		//size_t global_size = num_groups * local_size;
		size_t global_size = num_faces;

#define PRIM_BOUNDS_CPU
#define SCENE_BOUNDS_CPU
//#define GEN_MORTON_CPU
//#define SORT_CPU
#define GEN_NODES_CPU
#define REFIT_CPU

		// Prepare geometry data
		CalcPrimitiveBounds(num_vertices, num_faces, vertices, faces, centers, prim_bounds);
		// reduce scene bounds
		FindSceneBounds(num_faces, prim_bounds, scene_bounds);
		// calc_morton codes
		GenerateMortonCodes(num_faces, centers, scene_bounds, morton_codes);
		// Sort codes
		SortMortonCodes(num_faces, morton_codes, codes_sorted);
		// Generate hierachy
		GenerateNodes(num_faces, codes_sorted, prim_bounds, nodes, nodes_bounds);
		// Refit bounding boxes
		Refit(num_faces, nodes, nodes_bounds);

		// TODO copy nodes to readonly buffer
		/*
		auto out_nodes = TypedBuffer<SHARED::Node>(context, CL_MEM_READ_ONLY, num_nodes);
		auto out_bboxes = TypedBuffer<SHARED::AABB>(context, CL_MEM_READ_ONLY, num_nodes);
		queue.enqueueCopyBuffer(nodes.GetBuffer(), out_nodes.GetBuffer(), 0, 0, num_nodes);
		queue.enqueueCopyBuffer(bboxes.GetBuffer(), out_bboxes.GetBuffer(), 0, 0, num_nodes);
		*/
		queue.finish();
		return { nodes, nodes_bounds };
	}

	void BVHBuilder::CalcPrimitiveBounds(cl_uint num_vertices, cl_uint num_faces, const TypedBuffer<SHARED::Vertex>& vertices, const TypedBuffer<SHARED::Face>& faces, const TypedBuffer<cl_float3>& centers, const TypedBuffer<SHARED::AABB>& bboxes)
	{
#ifndef PRIM_BOUNDS_CPU

		m_kernel_prepare.setArg(0, sizeof(cl_uint), &num_vertices);
		m_kernel_prepare.setArg(1, sizeof(cl_uint), &num_faces);
		m_kernel_prepare.setArg(2, vertices.GetBuffer());
		m_kernel_prepare.setArg(3, faces.GetBuffer());
		m_kernel_prepare.setArg(4, centers.GetBuffer());
		m_kernel_prepare.setArg(5, bboxes.GetBuffer());
		cl_int err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_prepare, cl::NullRange, cl::NDRange(num_faces));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder prep]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
		
#else
		std::vector<SHARED::Vertex> _vertices = std::vector<SHARED::Vertex>(num_vertices);
		std::vector<SHARED::Face> _faces = std::vector<SHARED::Face>(num_faces);
		std::vector<cl_float3> _centers = std::vector<cl_float3>(num_faces);
		std::vector<SHARED::AABB> _bounds = std::vector<SHARED::AABB>(num_faces);

		auto queue = Compute::GetCommandQueue();
		queue.enqueueReadBuffer(vertices.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Vertex) * num_vertices, _vertices.data());
		queue.enqueueReadBuffer(faces.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Face) * num_faces, _faces.data());

		for (size_t i = 0; i < num_faces; i++) {
			auto face = _faces[i];
			auto v0 = _vertices[face.index.x].position;
			auto v1 = _vertices[face.index.y].position;
			auto v2 = _vertices[face.index.z].position;

			//printf("idx: %d, [%d,%d,%d]\n", i, face.index.x, face.index.y, face.index.z);

			glm::vec3 p0 = { v0.x,v0.y,v0.z };
			glm::vec3 p1 = { v1.x,v1.y,v1.z };
			glm::vec3 p2 = { v2.x,v2.y,v2.z };

			auto c = (p0 + p1 + p2) / 3.0f;
			_centers[i] = { c.x, c.y,c.z };

			glm::vec3 pmin = min(p0, min(p1, p2));
			glm::vec3 pmax = max(p0, max(p1, p2));
			SHARED::AABB bbox = {};
			bbox.min = { pmin.x,pmin.y,pmin.z };
			bbox.max = { pmax.x,pmax.y,pmax.z };
			_bounds[i] = bbox;
		}

		queue.enqueueWriteBuffer(centers.GetBuffer(), CL_TRUE, 0, sizeof(cl_float3) * num_faces, _centers.data());
		queue.enqueueWriteBuffer(bboxes.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::AABB) * num_faces, _bounds.data());
#endif // PRIM_BOUNDS_CPU

	}

	void BVHBuilder::FindSceneBounds(cl_uint num_primitives, const TypedBuffer<SHARED::AABB>& bboxes, const TypedBuffer<cl_float3>& scene_bounds)
	{
#ifndef SCENE_BOUNDS_CPU

		m_kernel_scene_bounds.setArg(0, sizeof(cl_uint), &num_primitives);
		m_kernel_scene_bounds.setArg(1, bboxes.GetBuffer());
		m_kernel_scene_bounds.setArg(2, scene_bounds.GetBuffer());
		cl_int err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_scene_bounds, cl::NullRange, cl::NDRange(32), cl::NDRange(32));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder scene bounds]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

#else

		std::vector<SHARED::AABB> _bounds = std::vector<SHARED::AABB>(num_primitives);
		auto queue = Compute::GetCommandQueue();
		queue.enqueueReadBuffer(bboxes.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::AABB) * num_primitives, _bounds.data());

		SHARED::AABB scene_bbox = {};
		scene_bbox.min = { 10000,10000,10000 };
		scene_bbox.max = { -10000,-10000,-10000 };
		for (auto bbox : _bounds) {
			scene_bbox = bbox_union(scene_bbox, bbox);
		}

		cl_float3 _scene_bounds[2];
		_scene_bounds[0] = scene_bbox.min;
		_scene_bounds[1] = scene_bbox.max;

		queue.enqueueWriteBuffer(scene_bounds.GetBuffer(), CL_TRUE, 0, sizeof(cl_float3) * 2, _scene_bounds);
#endif // !SCENE_BOUNDS_CPU
	}

	void BVHBuilder::GenerateMortonCodes(cl_uint num_primitives, const TypedBuffer<cl_float3>& centers, const TypedBuffer<cl_float3>& scene_bounds, const TypedBuffer<morton_key>& codes)
	{
#ifndef GEN_MORTON_CPU

		m_kernel_morton_code.setArg(0, sizeof(cl_uint), &num_primitives);
		m_kernel_morton_code.setArg(1, centers.GetBuffer());
		m_kernel_morton_code.setArg(2, scene_bounds.GetBuffer());
		m_kernel_morton_code.setArg(3, codes.GetBuffer());

		cl_int err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_morton_code, cl::NullRange, cl::NDRange(num_primitives));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder morton]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

#else
		auto _centers = std::vector<cl_float3>(num_primitives);
		auto _scene_bounds = std::vector<cl_float3>(2);
		auto _codes = std::vector<morton_key>(num_primitives);
		auto queue = Compute::GetCommandQueue();
		queue.enqueueReadBuffer(centers.GetBuffer(), CL_TRUE, 0, sizeof(cl_float3) * num_primitives, _centers.data());
		queue.enqueueReadBuffer(scene_bounds.GetBuffer(), CL_TRUE, 0, sizeof(cl_float3) * 2, _scene_bounds.data());

		glm::vec3 pmin = { _scene_bounds[0].x,_scene_bounds[0].y,_scene_bounds[0].z };
		glm::vec3 pmax = { _scene_bounds[1].x,_scene_bounds[1].y,_scene_bounds[1].z };
		glm::vec3 diagonal = pmax - pmin;

		for (int i = 0; i < num_primitives; i++) {
			glm::vec3 center = { _centers[i].x,_centers[i].y,_centers[i].z };
			const glm::vec p = (center - pmin) / diagonal;
			morton_key key = {};
			key.code = Float3ToInt64(p);
			key.index = i;
			_codes[i] = key;
		}

		queue.enqueueWriteBuffer(codes.GetBuffer(), CL_TRUE, 0, sizeof(morton_key) * num_primitives, _codes.data());

#endif // !GEN_MORTON_CPU
	}

	void BVHBuilder::SortMortonCodes(cl_uint num_primitives, const TypedBuffer<morton_key>& codes, TypedBuffer<morton_key>& codes_sorted)
	{
#ifndef SORT_CPU

		cl_int err;
		auto source_buffer = codes;
		auto target_buffer = codes_sorted;
		for (cl_uint shift = 0; shift < 64; shift += 8) {
			m_kernel_sort.setArg(0, sizeof(cl_uint), &num_primitives);
			m_kernel_sort.setArg(1, sizeof(cl_uint), &shift);
			m_kernel_sort.setArg(2, source_buffer.GetBuffer());
			m_kernel_sort.setArg(3, target_buffer.GetBuffer());

			err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_sort, cl::NullRange, cl::NDRange(32), cl::NDRange(32));
			auto tmp = source_buffer;
			source_buffer = target_buffer;
			target_buffer = tmp;
		}
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder sort]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}

		codes_sorted = source_buffer;
	
#else

		std::vector<morton_key> keys = std::vector<morton_key>(num_primitives);
		Compute::GetCommandQueue().enqueueReadBuffer(codes.GetBuffer(), CL_TRUE, 0, sizeof(morton_key) * num_primitives, keys.data());
		std::sort(keys.begin(), keys.end(), [](morton_key a, morton_key b) {return a.code < b.code; });
		Compute::GetCommandQueue().enqueueWriteBuffer(codes_sorted.GetBuffer(), CL_TRUE, 0, sizeof(morton_key) * num_primitives, keys.data());
#endif // !SORT_CPU
	}

	void BVHBuilder::GenerateNodes(cl_uint num_primitives, const TypedBuffer<morton_key>& codes, const TypedBuffer<SHARED::AABB>& prim_bounds, const TypedBuffer<SHARED::Node>& nodes, const TypedBuffer<SHARED::AABB>& nodes_bounds)
	{
#ifndef GEN_NODES_CPU

		m_kernel_hireachy.setArg(0, sizeof(cl_uint), &num_primitives);
		m_kernel_hireachy.setArg(1, codes.GetBuffer());
		m_kernel_hireachy.setArg(2, prim_bounds.GetBuffer());
		m_kernel_hireachy.setArg(3, nodes.GetBuffer());
		m_kernel_hireachy.setArg(4, nodes_bounds.GetBuffer());

		cl_int err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_hireachy, cl::NullRange, cl::NDRange(num_primitives));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder hireachy]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
#else
		size_t num_nodes = num_primitives * 2 - 1;
		std::vector<morton_key> _codes = std::vector<morton_key>(num_primitives);
		std::vector<SHARED::AABB> _prim_bounds = std::vector<SHARED::AABB>(num_primitives);
		std::vector<SHARED::Node> _nodes = std::vector<SHARED::Node>(num_nodes);
		std::vector<SHARED::AABB> _nodes_bounds = std::vector<SHARED::AABB>(num_nodes);

		auto queue = Compute::GetCommandQueue();
		queue.enqueueReadBuffer(codes.GetBuffer(), CL_TRUE, 0, sizeof(morton_key) * num_primitives, _codes.data());
		queue.enqueueReadBuffer(prim_bounds.GetBuffer(), CL_TRUE, 0, sizeof(morton_key) * num_primitives, _prim_bounds.data());

		int idx = generate(_codes.data(), _prim_bounds.data(), 0, num_primitives - 1, _nodes.data(), _nodes_bounds.data(), 0);

		queue.enqueueWriteBuffer(nodes.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Node) * num_nodes, _nodes.data());
		queue.enqueueWriteBuffer(nodes_bounds.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::AABB) * num_nodes, _nodes_bounds.data());
#endif // !GEN_NODES_CPU
	}

	void BVHBuilder::Refit(const cl_uint num_primitives, const TypedBuffer<SHARED::Node>& nodes, const TypedBuffer<SHARED::AABB>& bboxes)
	{
#ifndef REFIT_CPU
		cl_int err;
		cl::Buffer flags = cl::Buffer(Compute::GetContext(), CL_MEM_READ_WRITE, sizeof(cl_uint) * num_primitives - 1);
		err = Compute::GetCommandQueue().enqueueFillBuffer(flags, 0, 0, sizeof(cl_uint) * num_primitives - 1);
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder fill]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
		m_kernel_refit.setArg(0, sizeof(cl_uint), &num_primitives);
		m_kernel_refit.setArg(1, nodes.GetBuffer());
		m_kernel_refit.setArg(2, bboxes.GetBuffer());
		m_kernel_refit.setArg(3, flags);

		err = Compute::GetCommandQueue().enqueueNDRangeKernel(m_kernel_refit, cl::NullRange, cl::NDRange(num_primitives));
		if (err != CL_SUCCESS) {
			std::cout << "Error [BVHBuilder refit]: " << GET_CL_ERROR_CODE(err) << std::endl;
		}
#else
		size_t num_nodes = num_primitives * 2 - 1;
		std::vector<SHARED::Node> _nodes = std::vector<SHARED::Node>(num_nodes);
		std::vector<SHARED::AABB> _nodes_bounds = std::vector<SHARED::AABB>(num_nodes);
		auto queue = Compute::GetCommandQueue();
		queue.enqueueReadBuffer(nodes.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Node) * num_nodes, _nodes.data());
		queue.enqueueReadBuffer(bboxes.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::AABB) * num_nodes, _nodes_bounds.data());

		calc_bboxes(_nodes.data(), _nodes_bounds.data(), 0);

		queue.enqueueWriteBuffer(bboxes.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::AABB) * num_nodes, _nodes_bounds.data());
#endif // !REFIT_CPU
	}
	inline uint32_t BVHBuilder::findSplit(const morton_key* codes, const uint32_t first, const uint32_t last)
	{
		const uint64_t firstCode = codes[first].code;
		const uint64_t lastCode = codes[last].code;

		if (firstCode == lastCode)
			return (first + last) >> 1;

		// Calculate the number of highest bits that are the same
		// for all objects, using the count-leading-zeros intrinsic.

		const uint64_t commonPrefix = clz64(firstCode ^ lastCode);
		//int commonPrefix = __clz(firstCode ^ lastCode);

		// Use binary search to find where the next bit differs.
		// Specifically, we are looking for the highest object that
		// shares more than commonPrefix bits with the first one.

		uint64_t split = first; // initial guess
		uint64_t step = last - first;

		do
		{
			step = (step + 1) >> 1; // exponential decrease
			uint64_t newSplit = split + step; // proposed new position

			if (newSplit < last)
			{
				const uint64_t splitCode = codes[newSplit].code;
				uint64_t splitPrefix = clz64(firstCode ^ splitCode);
				if (splitPrefix > commonPrefix)
					split = newSplit; // accept proposal
			}
		} while (step > 1);

		return static_cast<uint32_t>(split);
	}
	uint32_t BVHBuilder::generate(const morton_key* codes, const SHARED::AABB* bounds, const uint32_t first, const uint32_t last, SHARED::Node* nodes, SHARED::AABB* nodes_bounds, const uint32_t idx)
	{
		if (first == last) {
			SHARED::Node node = {};
			int index = codes[first].index;
			node.left = -1;
			node.right = index;
			nodes[idx] = node;
			//printf("idx: %d, leaf\n", idx);
			nodes_bounds[idx] = bounds[index];
			return 1;
		}
		// find split
		int split = findSplit(codes, first, last);
		//int range = last - first;
		//int split = first + (range / 2);


		// process sub-ranges
		uint32_t num_left = generate(codes, bounds, first, split, nodes, nodes_bounds, idx + 1);
		uint32_t num_right = generate(codes, bounds, split + 1, last, nodes, nodes_bounds, idx + num_left + 1);

		SHARED::Node node = {};
		node.left = idx + 1;
		node.right = num_left + idx + 1;
		nodes[idx] = node;

		return num_left + num_right + 1;
	}

	

	SHARED::AABB BVHBuilder::calc_bboxes(SHARED::Node* nodes, SHARED::AABB* nodes_bboxes, uint32_t idx)
	{
		SHARED::Node& node = nodes[idx];
		SHARED::AABB& node_bbox = nodes_bboxes[idx];
		// if leaf
		if (node.left == -1) {
			return node_bbox;
		}

		// internal node
		uint32_t left = node.left;
		uint32_t right = node.right;
		SHARED::AABB bbox = bbox_union(calc_bboxes(nodes, nodes_bboxes, left), calc_bboxes(nodes, nodes_bboxes, right));
		node_bbox = bbox;

		return bbox;
	}
}