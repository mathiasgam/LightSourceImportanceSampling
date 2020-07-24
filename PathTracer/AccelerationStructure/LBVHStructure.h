#pragma once

#include "AccelerationStructure.h"

namespace LSIS {

	class LBVHStructure : public AccelerationStructure {
	public:
		LBVHStructure();
		virtual ~LBVHStructure();

		virtual void Build(const VertexData* vertices, size_t num_vertices, const uint32_t* indices, size_t num_indices) override;
		virtual void TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer) override;

	private:

		// Defines types for the buffers
		struct Vertex {
			cl_float4 position;
			cl_float4 normal;
			cl_float4 uv;
		};

		struct Face {
			cl_uint4 index;
		};

		struct Node {
			cl_float4 min; // .w is left neighbor
			cl_float4 max; // .w is right neighbor
		};

		void CompileKernels();
		void LoadGeometryBuffers(const Vertex* vertices, size_t num_vertices, const Face* faces, size_t num_faces);
		void LoadBVHBuffer(const Node* nodes, size_t num_nodes);

	private:

		bool isBuild = false;

		cl::Program m_program;
		cl::Kernel m_kernel;

		cl::Buffer m_buffer_bvh;
		cl::Buffer m_buffer_faces;
		cl::Buffer m_buffer_vertices;

		cl_uint m_num_vertices;
		cl_uint m_num_faces;
		cl_uint m_num_nodes;

	};
}