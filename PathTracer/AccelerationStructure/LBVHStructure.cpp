#include "pch.h"
#include "LBVHStructure.h"

#include <intrin.h>
#include "DataStructures/MortonCode.h"

namespace LSIS {

	LBVHStructure::LBVHStructure()
	{
		CompileKernels();
	}

	LBVHStructure::~LBVHStructure()
	{
	}

	inline float max(const glm::vec3& vec) {
		return vec.x > vec.y ? (vec.x > vec.z ? vec.x : vec.z) : (vec.y > vec.z ? vec.y : vec.z);
	}

	inline uint64_t clz64(const uint64_t i) {
		return __lzcnt64(i);
	}

	inline uint64_t sigma(const uint64_t i, const uint64_t j) {
		return clz64(i ^ j);
	}

	inline glm::vec3 mad(const glm::vec3& a, const glm::vec3& b, const glm::vec3& c) {
		return (a * b) + c;
	}

	/// implementation from https://devblogs.nvidia.com/thinking-parallel-part-iii-tree-construction-gpu/
	inline uint32_t LBVHStructure::findSplit(const morton_code_64_t* codes, const uint32_t first, const uint32_t last) {
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

	uint32_t LBVHStructure::generate(const morton_code_64_t* codes, const uint32_t first, const uint32_t last, Node* nodes, const uint32_t idx) {

		if (first == last) {
			Node node = {};
			node.min.w = -1;
			node.max.w = static_cast<float>(codes[first].index);
			nodes[idx] = node;
			//printf("idx: %d, leaf\n", idx);
			return 1;
		}
		// find split
		int split = findSplit(codes, first, last);
		//int range = last - first;
		//int split = first + (range / 2);


		// process sub-ranges
		uint32_t num_left = generate(codes, first, split, nodes, idx + 1);
		uint32_t num_right = generate(codes, split + 1, last, nodes, idx + num_left + 1);

		Node node = {};
		node.min.w = static_cast<float>(idx + 1);
		node.max.w = static_cast<float>(num_left + idx + 1);
		nodes[idx] = node;

		return num_left + num_right + 1;
	}

	LBVHStructure::AABB LBVHStructure::calc_bboxes(Node* nodes, const AABB* bboxes, uint32_t idx) {
		Node& node = nodes[idx];

		// if leaf
		if (node.min.w == -1) {
			uint32_t face_idx = static_cast<uint32_t>(node.max.w);
			const AABB& bbox = bboxes[face_idx];
			node.min.x = bbox.p_min.x;
			node.min.y = bbox.p_min.y;
			node.min.z = bbox.p_min.z;
			node.max.x = bbox.p_max.x;
			node.max.y = bbox.p_max.y;
			node.max.z = bbox.p_max.z;
			return bbox;
		}

		// internal node
		uint32_t left = static_cast<uint32_t>(node.min.w);
		uint32_t right = static_cast<uint32_t>(node.max.w);
		AABB bbox = AABB(calc_bboxes(nodes, bboxes, left), calc_bboxes(nodes, bboxes, right));
		node.min.x = bbox.p_min.x;
		node.min.y = bbox.p_min.y;
		node.min.z = bbox.p_min.z;
		node.max.x = bbox.p_max.x;
		node.max.y = bbox.p_max.y;
		node.max.z = bbox.p_max.z;

		return bbox;
	}
	   
	void LBVHStructure::Build(const VertexData* in_vertices, size_t num_vertices, const uint32_t* in_indices, size_t num_indices)
	{
		std::cout << "Building LBVH\n";
		std::cout << "Num Vertices: " << num_vertices << std::endl;
		std::cout << "Num Indices: " << num_indices << std::endl;

		const size_t N = num_indices / 3;

		m_num_faces = N;
		m_num_vertices = num_vertices;

		Face* faces = new Face[N]; // (Face*)malloc(sizeof(Face) * N);
		Vertex* vertices = new Vertex[num_vertices]; // (Vertex*)malloc(sizeof(Vertex) * num_vertices);

		// exstract bounding boxes for each face in the scene
		AABB * bboxes = new AABB[N];// (AABB*)malloc(sizeof(AABB) * N);
		glm::vec3* centers = new glm::vec3[N]; // (glm::vec3*) malloc(sizeof(glm::vec3) * N);

		// Parse vertices
		for (size_t i = 0; i < num_vertices; i++) {
			const VertexData& vtx = in_vertices[i];
			Vertex v{};
			v.position.x = vtx.position[0];
			v.position.y = vtx.position[1];
			v.position.z = vtx.position[2];
			v.position.w = 0.0f;
			v.normal.x = vtx.normal[0];
			v.normal.y = vtx.normal[1];
			v.normal.z = vtx.normal[2];
			v.normal.w = 0.0f;
			v.uv.x = vtx.uv[0];
			v.uv.y = vtx.uv[1];
			v.uv.z = 0.0f;
			v.uv.w = 0.0f;
			vertices[i] = v;
		}

		// Parse faces
		for (size_t i = 0; i < N; i++) {
			size_t index = i * 3;
			Face f{};
			f.index.x = in_indices[index + 0];
			f.index.y = in_indices[index + 1];
			f.index.z = in_indices[index + 2];
			f.index.w = 0U;
			faces[i] = f;
		}

		// Find scene bounds
		AABB bounds = AABB();
		for (size_t i = 0; i < N; i++) {
			const Face& face = faces[i];

			const Vertex& v0 = vertices[face.index.x];
			const Vertex& v1 = vertices[face.index.y];
			const Vertex& v2 = vertices[face.index.z];

			glm::vec3 p0 = { v0.position.x, v0.position.y, v0.position.z };
			glm::vec3 p1 = { v1.position.x, v1.position.y, v1.position.z };
			glm::vec3 p2 = { v2.position.x, v2.position.y, v2.position.z };

			// find center of triangle as the average position
			centers[i] = (p0 + p1 + p2) / 3.0f;
			AABB bbox = AABB(p0);
			bbox.add_AABB(p1);
			bbox.add_AABB(p2);

			bboxes[i] = bbox;

			bounds.add_AABB(bbox);
		}

		// calculate transforms to project the coordinates into the range [0,1]
		glm::vec3 diagonal = bounds.p_max - bounds.p_min;
		const float max_dim = max(diagonal);
		const glm::vec3 scale = glm::vec3(1.0f / max_dim);
		const glm::vec3 transform = -bounds.p_min * scale;

		// create temporary array on the stack for building the bvh structure
		morton_code_64_t* morton_keys = new morton_code_64_t[N]; // (morton_code_64_t*)malloc(sizeof(morton_code_64_t) * N);

		// generate morton codes based on the bounding boxes in the scene
		for (int i = 0; i < N; i++) {
			const glm::vec p = mad(centers[i], scale, transform);
			morton_code_64_t key{};
			key.code = Float3ToInt64(p);
			key.index = i;
			morton_keys[i] = key;
		}

		// sort morton codes
		std::sort(morton_keys, morton_keys + N);

		// internal nodes is equal to N-1 and where N is the amount of leaves.
		size_t num_internal_nodes = N - 1;
		m_num_nodes = N + num_internal_nodes;

		// clear and resize the nodes array to hold the maximum amounds of nodes, based on the number of leaves
		Node* nodes = new Node[m_num_nodes]; // (Node*)malloc(sizeof(Node) * m_num_nodes);

		// Generate Hiearachy
		generate(morton_keys, 0, static_cast<uint32_t>(N - 1), nodes, 0);

		// Calculate the bounding boxes
		calc_bboxes(nodes, bboxes, 0);

		// Upload data to the GPU
		LoadBVHBuffer(nodes, m_num_nodes);
		LoadGeometryBuffers(vertices, m_num_vertices, faces, m_num_faces);

		delete[] faces;
		delete[] vertices;
		delete[] bboxes;
		delete[] centers;
		delete[] morton_keys;
	}

	void LBVHStructure::TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer)
	{
		if (!isBuild) {
			std::cout << "Error: BVH not yet build!\n";
			return;
		}

		// Get ray count;
		cl_uint num_rays = static_cast<cl_uint>(ray_buffer.Count());

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