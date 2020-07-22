#pragma once

#include "AccelerationStructure.h"

namespace LSIS::Compute {

	struct Node {

	};

	class LBVHStructure : public AccelerationStructure{
	public:
		LBVHStructure();
		virtual ~LBVHStructure();

		virtual void Build(const VertexData* vertices, size_t num_vertices, const uint32_t* indices, size_t num_indices) override;
		virtual void TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer) override;

	private:
		void CompileKernels();
		void LoadGeometryBuffers();
		void LoadBVHBuffer();

	private:
		cl::Program m_program;
		cl::Kernel m_kernel;

		cl::Buffer m_buffer_bvh;
		cl::Buffer m_buffer_faces;
		cl::Buffer m_buffer_vertices;
	};
}