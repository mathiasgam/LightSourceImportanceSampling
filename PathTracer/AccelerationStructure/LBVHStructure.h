#pragma once

#include "AccelerationStructure.h"

namespace LSIS::Compute {

	struct Node {

	};

	struct Vertex {

	};

	struct Face {

	};

	class LBVHStructure : public AccelerationStructure{
	public:
		LBVHStructure();
		virtual ~LBVHStructure();

		virtual void Build(std::vector<Mesh>& meshes) override;
		virtual void TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer) override;

	private:
		cl_program m_program;
		cl_kernel m_kernel;

		//ObjectBuffer<Node> m_nodes;
		//ObjectBuffer<Vertex> m_vertices;
		//ObjectBuffer<Face> m_faces;
	};
}