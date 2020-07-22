#pragma once

#include "Compute/Buffer.h"
#include "../Assets/Kernels/shared_defines.h"

#include "Mesh/Mesh.h"

namespace LSIS::Compute {

	struct Vertex {
		float4 position;
		float4 normal;
		float4 uv;
	};

	struct Face {
		uint4 indices;
	};

	using RayBuffer = TypedBuffer<SHARED::Ray>;
	using IntersectionBuffer = TypedBuffer<SHARED::Intersection>;

	class AccelerationStructure {
	public:

		virtual void Build(const VertexData* vertices, size_t num_vertices, const uint32_t* indices, size_t num_indices) = 0;
		virtual void TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer) = 0;

	private:
	};

}