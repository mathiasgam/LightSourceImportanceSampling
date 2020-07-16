#pragma once

#include "Compute/Buffer.h"

#include "Mesh/Mesh.h"

namespace LSIS::Compute {

	struct Ray {
		cl_float4 origin;
		cl_float4 direction;
	};

	struct Intersection {
		cl_float4 position;
		cl_float4 normal;
		cl_float4 incoming;
	};

	using RayBuffer = TypedBuffer<Ray>;
	using IntersectionBuffer = TypedBuffer<Intersection>;

	class AccelerationStructure {
	public:

		virtual void Build(std::vector<Mesh>& meshes) = 0;
		virtual void TraceRays(RayBuffer& ray_buffer, IntersectionBuffer& intersection_buffer) = 0;

	private:
	};

}