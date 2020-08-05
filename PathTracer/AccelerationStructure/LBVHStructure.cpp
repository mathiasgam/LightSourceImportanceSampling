#include "pch.h"
#include "LBVHStructure.h"

#include <intrin.h>
#include "DataStructures/MortonCode.h"

#include "Core/Timer.h"

namespace LSIS {

	LBVHStructure::LBVHStructure()
	{
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

	uint32_t LBVHStructure::generate(const morton_code_64_t* codes, const uint32_t first, const uint32_t last, SHARED::Node* nodes, const uint32_t idx) {

		if (first == last) {
			SHARED::Node node = {};
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

		SHARED::Node node = {};
		node.min.w = static_cast<float>(idx + 1);
		node.max.w = static_cast<float>(num_left + idx + 1);
		nodes[idx] = node;

		return num_left + num_right + 1;
	}

	LBVHStructure::AABB LBVHStructure::calc_bboxes(SHARED::Node* nodes, const AABB* bboxes, uint32_t idx) {
		SHARED::Node& node = nodes[idx];

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
		PROFILE_SCOPE("LBVH Build");

		if (num_indices == 0 || num_vertices == 0) {
			return;
		}

		const size_t N = num_indices / 3;

		m_num_faces = N;
		m_num_vertices = num_vertices;

		SHARED::Face* faces = new SHARED::Face[N]; // (Face*)malloc(sizeof(Face) * N);
		SHARED::Vertex* vertices = new SHARED::Vertex[num_vertices]; // (Vertex*)malloc(sizeof(Vertex) * num_vertices);

		// exstract bounding boxes for each face in the scene
		AABB * bboxes = new AABB[N];// (AABB*)malloc(sizeof(AABB) * N);
		glm::vec3* centers = new glm::vec3[N]; // (glm::vec3*) malloc(sizeof(glm::vec3) * N);

		// Parse vertices
		for (size_t i = 0; i < num_vertices; i++) {
			const VertexData& vtx = in_vertices[i];
			SHARED::Vertex v{};
			v.position.x = vtx.position[0];
			v.position.y = vtx.position[1];
			v.position.z = vtx.position[2];
			v.position.w = vtx.uv[0];
			v.normal.x = vtx.normal[0];
			v.normal.y = vtx.normal[1];
			v.normal.z = vtx.normal[2];
			v.normal.w = vtx.uv[1];
			vertices[i] = v;
		}

		// Parse faces
		for (size_t i = 0; i < N; i++) {
			size_t index = i * 3;
			SHARED::Face f{};
			f.index.x = in_indices[index + 0];
			f.index.y = in_indices[index + 1];
			f.index.z = in_indices[index + 2];
			f.index.w = 0U; // material index

			faces[i] = f;
		}

		// Find scene bounds
		AABB bounds = AABB();
		for (size_t i = 0; i < N; i++) {
			const SHARED::Face& face = faces[i];

			const SHARED::Vertex& v0 = vertices[face.index.x];
			const SHARED::Vertex& v1 = vertices[face.index.y];
			const SHARED::Vertex& v2 = vertices[face.index.z];

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
		SHARED::Node* nodes = new SHARED::Node[m_num_nodes]; // (Node*)malloc(sizeof(Node) * m_num_nodes);

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

		isBuild = true;
	}

	void LBVHStructure::LoadGeometryBuffers(const SHARED::Vertex* vertices, size_t num_vertices, const SHARED::Face* faces, size_t num_faces)
	{
		// initialize buffers
		m_buffer_vertices = TypedBuffer<SHARED::Vertex>(Compute::GetContext(), CL_MEM_READ_ONLY, num_vertices);
		m_buffer_faces = TypedBuffer<SHARED::Face>(Compute::GetContext(), CL_MEM_READ_ONLY, num_faces);
		m_num_vertices = num_vertices;
		m_num_faces = num_faces;

		// Upload data
		auto queue = Compute::GetCommandQueue();
		queue.enqueueWriteBuffer(m_buffer_vertices.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Vertex) * num_vertices, static_cast<const void*>(vertices));
		queue.enqueueWriteBuffer(m_buffer_faces.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Face) * num_faces, static_cast<const void*>(faces));
	}

	void LBVHStructure::LoadBVHBuffer(const SHARED::Node* nodes, size_t num_nodes)
	{
		// initialize buffers
		m_buffer_bvh = TypedBuffer<SHARED::Node>(Compute::GetContext(), CL_MEM_READ_ONLY, num_nodes);
		m_num_nodes = num_nodes;

		// Upload data
		auto queue = Compute::GetCommandQueue();
		queue.enqueueWriteBuffer(m_buffer_bvh.GetBuffer(), CL_TRUE, 0, sizeof(SHARED::Node) * num_nodes, static_cast<const void*>(nodes));

	}

}